# Cowcium ML Server

Python FastAPI service for calcium inference using:

- `svm_model.pkl`
- `scaler.pkl`

The service can either:

- run prediction on a provided sensor reading
- fetch a live stabilized reading from the C++ sensor server and predict from that

## Endpoints

- `GET /health`
- `POST /predict`
- `GET /predict-live`

## Environment variables

- `COWCIUM_SENSOR_BASE_URL`
  - default: `http://127.0.0.1:8080`
- `COWCIUM_SCALER_PATH`
  - default: `./scaler.pkl`
- `COWCIUM_MODEL_PATH`
  - default: `./svm_model.pkl`

## Run

```bash
pip install -r requirements.txt
./run_server.sh
```
