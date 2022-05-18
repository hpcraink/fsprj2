Prometheus

Metric name      Label                     Sample  
Temperature      location=outside    90

- Multiple labels possible
- &lt;metric name&gt; {&lt;label name&gt;=&lt;label value&gt;, ...}
- Prometheus collects metrics from monitored targets by scraping metrics HTTP endpoints
- A single prometheus server is able to ingest up to one million samples per second as several million time series

**Configuration file**

- Can be changed and applied without having to restart prometheus
- Reload can be done by executing **kill -SIGHUP &lt;pid&gt;**
- You can also pass parameters (flags) at startup time to ./prometheus
    - Those cannot be changed without restarting Prometheus
- --config.file=config.yml

**Configuration file example**

**# my global config  
global:  
    scrape_interval:       15s  #Default is 1 minute  
    evaluation_interval: 15s  #Default is 1 minute  
\# Alertmanager configuration  
alerting:  
     alertmanagers:  
     \- static_configs:  
        \- targets:  
           # -alertmanager:9093

- Default configuration is loaded to scrape metrics from prometheus itself
    
- localhost:9090/metrics --> metrics that prometheus scrapes itself
    
- localhost:9090/config
    

**Exposition formats**

- Simple text-based format

**4 types of metrics**

- Counter
    - A value that goes up (e.g. visits to a webiste)
- Gauge
    - Single numeric value that can go up and down (e.g. CPU load, temperature)
- Histogram
    - Samples observations (e.g. request durations or response sizes) and these observations get counted into **buckets**. Includes (\_count and \_sum)
    - Main purpose is calculating quantiles
- Summary
    - Similar to a histogram, a summary samples observations (e.g. request durations or response sizes). A summary also provides a total count of observations and a sum of all observerd values, it calculates configurable quantiles over a sliding time window.
    - Example: You need 2 counters for calculating the latency 1) total request(\_count) 2) the total latency of those requests (\_sum)  
        Take the rate() and divide = average latency

**Push Gateway**

- Sometimes metrics cannot be scraped
- Used as an intermediary service
- Pushgateway never forgets the metrics unless they are deleted via the api

![4dd892fd7154519e82c91da78d01a360.png](/_resources/517b68ef9fac4d139d3fcd1b15572b36.png)

- push\_to\_gateway: replaces metrics with the same grouping key
- pushadd\_to\_gateway: only replaces metrics with the same name and grouping key
- delete\_from\_gateway: deltes metrics with the given job and grouping key

**Querying**

- PromQL is read-only
- **Instant vector:** a set of time series containing a single sample for each time series, all sharing the same timestamp; Example: node\_cpu\_seconds_total
- **Range vector: **a set of time series containing a range of data points over time for each time series; Example: node\_cpu\_seconds_total\[5m\]
- **Scalar**: a simple numeric floating point value; Example: -3.14
- **String**: a simple string value; currently unused; Example: foobar

**Querying Demo**

- up{job='prometheus'}
- Look at exmaples at prometheus.io

**Service Discovery   
**

- Works with Amazon EC2
- Watches for certain labels
- Alternatively you can use a file like target.json in which prometheus looks up regulary
