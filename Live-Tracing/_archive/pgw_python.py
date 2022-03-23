import sys

numbersList = [float(sys.argv[i]) for i in range(1, len(sys.argv))]
from prometheus_client import CollectorRegistry, Gauge, push_to_gateway

registry = CollectorRegistry()
g = Gauge('job_last_success_unixtime', 'Last time the course batch job has finished', registry=registry)
g.set(numbersList[0])
push_to_gateway('localhost:9091', job='batchA', registry=registry)
