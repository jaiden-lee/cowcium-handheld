from __future__ import annotations

import logging
import os
from pathlib import Path
from typing import Any

import joblib
import numpy as np
import requests
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel


logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s",
)
logger = logging.getLogger("cowcium-ml-server")


SENSOR_BASE_URL = os.getenv("COWCIUM_SENSOR_BASE_URL", "http://127.0.0.1:8080")
SCALER_PATH = Path(os.getenv("COWCIUM_SCALER_PATH", "./scaler.pkl"))
MODEL_PATH = Path(os.getenv("COWCIUM_MODEL_PATH", "./svm_model.pkl"))


app = FastAPI(title="Cowcium ML Server")

scaler: Any = None
svm_model: Any = None


class SensorReading(BaseModel):
    red: float
    green: float
    blue: float
    clear: float


class PredictionResponse(BaseModel):
    predicted_class: int
    is_healthy: bool
    label: str
    features: dict[str, float]
    reading: SensorReading


class FeatureResponse(BaseModel):
    features: dict[str, float]
    reading: SensorReading


def load_model_artifacts() -> None:
    global scaler, svm_model

    if not SCALER_PATH.exists():
        raise FileNotFoundError(f"Scaler file not found: {SCALER_PATH}")

    if not MODEL_PATH.exists():
        raise FileNotFoundError(f"Model file not found: {MODEL_PATH}")

    scaler = joblib.load(SCALER_PATH)
    svm_model = joblib.load(MODEL_PATH)


def build_feature_vector(reading: SensorReading) -> tuple[np.ndarray, dict[str, float]]:
    if reading.green == 0 or reading.blue == 0:
        raise ValueError("Green and blue values must be non-zero for feature generation")

    r_over_g = reading.red / reading.green
    g_over_b = reading.green / reading.blue
    r_over_b = reading.red / reading.blue

    features = {
        "r_over_g": r_over_g,
        "g_over_b": g_over_b,
        "r_over_b": r_over_b,
    }
    vector = np.array([[r_over_g, g_over_b, r_over_b]], dtype=float)
    return vector, features


def run_prediction(reading: SensorReading) -> PredictionResponse:
    if scaler is None or svm_model is None:
        raise RuntimeError("Model artifacts are not loaded")

    vector, features = build_feature_vector(reading)
    scaled_vector = scaler.transform(vector)
    prediction = svm_model.predict(scaled_vector)[0]

    logger.info(
        "event=prediction predicted_class=%s red=%s green=%s blue=%s clear=%s r_over_g=%s g_over_b=%s r_over_b=%s",
        prediction,
        reading.red,
        reading.green,
        reading.blue,
        reading.clear,
        features["r_over_g"],
        features["g_over_b"],
        features["r_over_b"],
    )

    predicted_class = int(prediction)
    is_healthy = predicted_class == 1
    label = "healthy" if is_healthy else "subclinical_hypocalcemia"

    return PredictionResponse(
        predicted_class=predicted_class,
        is_healthy=is_healthy,
        label=label,
        features=features,
        reading=reading,
    )


def fetch_live_reading() -> SensorReading:
    try:
        response = requests.get(f"{SENSOR_BASE_URL}/record-reading", timeout=45)
        response.raise_for_status()
        data = response.json()
        return SensorReading(
            red=float(data["red"]),
            green=float(data["green"]),
            blue=float(data["blue"]),
            clear=float(data["clear"]),
        )
    except requests.RequestException as exc:
        raise HTTPException(status_code=502, detail=f"Sensor server request failed: {exc}") from exc
    except (KeyError, ValueError, TypeError) as exc:
        raise HTTPException(status_code=502, detail=f"Invalid sensor response: {exc}") from exc


@app.on_event("startup")
def startup_event() -> None:
    load_model_artifacts()
    logger.info(
        "event=startup scaler_path=%s model_path=%s sensor_base_url=%s",
        SCALER_PATH,
        MODEL_PATH,
        SENSOR_BASE_URL,
    )


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}


@app.post("/predict", response_model=PredictionResponse)
def predict(reading: SensorReading) -> PredictionResponse:
    try:
        return run_prediction(reading)
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    except RuntimeError as exc:
        raise HTTPException(status_code=500, detail=str(exc)) from exc


@app.post("/features", response_model=FeatureResponse)
def features(reading: SensorReading) -> FeatureResponse:
    try:
        _, feature_values = build_feature_vector(reading)
        return FeatureResponse(features=feature_values, reading=reading)
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@app.get("/predict-live", response_model=PredictionResponse)
def predict_live() -> PredictionResponse:
    try:
        reading = fetch_live_reading()
        return run_prediction(reading)
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    except RuntimeError as exc:
        raise HTTPException(status_code=500, detail=str(exc)) from exc
