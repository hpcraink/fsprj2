import time
import requests
import os
from dotenv import load_dotenv


class SwitchModel:
    def __init__(self, id, apower, voltage, current, aenergy_total, aenergy_by_minute):
        self.id = id
        self.apower = apower
        self.voltage = voltage
        self.current = current
        self.aenergy_total = aenergy_total
        self.aenergy_by_minute = aenergy_by_minute


def request_data_from_shelly(ip, device_id) -> SwitchModel | None:
    try:
        response = requests.get(f"http://{ip}/rpc/Switch.GetStatus?id={device_id}")
        if response.status_code == 200:
            switch_0_model = response.json()

            switch_instance = SwitchModel(
                id=switch_0_model.get("id"),
                apower=switch_0_model.get("apower"),  # Watt
                voltage=switch_0_model.get("voltage"),  # Volt
                current=switch_0_model.get("current"),  # Amperes
                aenergy_total=switch_0_model.get("aenergy", {}).get("total"),
                aenergy_by_minute=[
                    switch_0_model.get("aenergy", {}).get("by_minute", [])[0],
                    switch_0_model.get("aenergy", {}).get("by_minute", [])[1],
                    switch_0_model.get("aenergy", {}).get("by_minute", [])[2]
                ]
            )
            return switch_instance
        else:
            print(f"Fehler bei der Anfrage. Status-Code: {response.status_code}")

    except Exception as e:
        print(f"Fehler: {e}")

    return None


def send_data_to_influx(switch_instance) -> None:
    IOTRACE_DATABASE_IP = os.getenv("IOTRACE_DATABASE_IP")
    IOTRACE_DATABASE_PORT = os.getenv("IOTRACE_DATABASE_PORT")

    influxdb_url = f"http://{IOTRACE_DATABASE_IP}:{IOTRACE_DATABASE_PORT}/api/v2/write"
    bucket = os.getenv("IOTRACE_INFLUX_BUCKET")
    org = os.getenv("IOTRACE_INFLUX_ORGANIZATION")
    token = os.getenv("IOTRACE_INFLUX_TOKEN")

    data = f"shelly_power_measurement,id={switch_instance.id} apower_watt={switch_instance.apower},voltage={switch_instance.voltage},amper={switch_instance.current},aenergy={switch_instance.aenergy_total},aenergy_by_minute_0={switch_instance.aenergy_by_minute[0]},aenergy_by_minute_1={switch_instance.aenergy_by_minute[1]},aenergy_by_minute_2={switch_instance.aenergy_by_minute[2]}"

    params = {
        "bucket": bucket,
        "precision": "ns",
        "org": org,
    }

    headers = {
        "Authorization": f"Token {token}"
    }

    try:
        response = requests.post(influxdb_url, params=params, data=data, headers=headers)

        if response.status_code == 204:
            print(f"Daten erfolgreich in die InfluxDB geschrieben. => {data}")
        else:
            print(f"Fehler beim Schreiben der Daten. Status-Code: {response.status_code} => {response.text}")

    except Exception as e:
        print(f"Fehler: {e}")


if __name__ == '__main__':
    load_dotenv()

    while True:
        start_time = time.time()

        switch_instance = request_data_from_shelly("192.168.178.96", 0)
        if switch_instance:
            send_data_to_influx(switch_instance)

        end_time = time.time()
        execution_time = end_time - start_time
        time_to_sleep = max(0, 1 - execution_time)
        time.sleep(time_to_sleep)
