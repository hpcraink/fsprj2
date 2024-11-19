# run with: $ python3 influxtest.py

import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS
import pandas as pd
import numpy as np
from datetime import datetime, timedelta, timezone
import time

org = "hse"
token = "OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng=="
url="http://127.0.0.1:8086"

client = influxdb_client.InfluxDBClient(
   url=url,
   token=token,
   org=org
)
query_api = client.query_api()
write_api = client.write_api(write_options=SYNCHRONOUS)

time_range="300d"

# needed globally for dashboard url creation
first_timestamp = ""
last_timestamp = ""

# Lists of all functions changing cursor offsets indirectly - See page 31
function_open_list = [
  "fopen", "fopen64",
  # MPI
  "MPI_File_open"
]
function_close_list = [
  "close", "fclose",
  # MPI
  "MPI_File_close"
]


# Lists of all function names changing the cursor offset - See page 36
# Row 1
function_read_list = [
  "fgetc", "fgetc_unlocked",
  "fgets", "fgets_unlocked",
  "fgetwc", "fgetwc_unlocked",
  "fgetws", "fgetws_unlocked",
  "fread", "fread_unlocked",
  "getc", "getc_unlocked",
  "getdelim",
  "getline",
  "getw",
  "getwc", "getwc_unlocked",
  "read",
  "readv",
  # MPI
  "MPI_File_read"
]
# Row 2
function_read_list_param = [
  "pread", "pread64",
  "preadv", "preadv64"
]

# Row 3
function_write_list = [
  "fprintf",
  "fputc", "fputc_unlocked",
  "fputs", "fputs_unlocked",
  "fputwc", "fputwc_unlocked",
  "fputws", "fputws_unlocked",
  "fwprintf",
  "fwrite", "fwrite_unlocked",
  "putc", "putc_unlocked",
  "putw",
  "putwc", "putwc_unlocked",
  "vfprintf", 
  "vfwprintf",
  "write",
  "writev",
  # MPI
  "MPI_File_write"
]
# Row 4
function_write_list_param = [
  "pwrite", "pwrite64",
  "pwritev", "pwritev64"
]

# Row 5
function_seek_list = [
  "ftell",
  "ftello", "ftello64",
  "lseek", "lseek64",
  # MPI
  "MPI_File_seek"
]
# Row 11
function_seek2_list = [
  "fseek",
  "fseeko", "fseeko64"
]

# Row 6
function_read_list_param_v2 = [
  "preadv2", "preadv64v2"
]
# Row 7
function_write_list_param_v2 = [
  "pwritev2", "pwritev64v2"
]

# Row 8
function_mmap_list = [
  "mmap", "mmap64"
]
# Row 9
function_sync_list = [
  "msync", "munmap"
]
# Row 10
function_unget_list = [
  "ungetc", "ungetwc"
]

# Row 12
function_rewind_list = [
  "rewind"
]
# Row 13
function_purge_list = [
  "__purge"
]

# ADDITIONAL MPI FUNCTIONS
# seek & io
function_mpi_io_at_list = [
  "MPI_File_read_at",
  "MPI_File_write_at"
]

# shared file pointer
function_mpi_shared_list = [
  "MPI_File_read_shared",
  "MPI_File_write_shared"
]

# collective io
# seek & io
function_mpi_shared_list = [
  "MPI_File_read_at_all",
  "MPI_File_write_at_all"
]

# shared file pointer
function_mpi_shared_list = [
  "MPI_File_read_ordered",
  "MPI_File_write_ordered"
]



def get_fluxtable_by_jobname_and_filename(jobname, filename, bucket="hsebucket", measurement="libiotrace"):
  query = f'''
    from(bucket: "{bucket}")
      |> range(start: -{time_range})
      |> filter(fn: (r) => r._measurement == "{measurement}")
      |> filter(fn: (r) => r.jobname == "{jobname}")
      |> filter(fn: (r) => r.filename == "{filename}")
  '''
  fluxtables = query_api.query(org=org, query=query)
  return fluxtables


def write_data_to_influx(dataframe, new_measurement_name, bucket="hsebucket", field_name="cursor_position", field_column="offset_after"):
  for index, row in dataframe.iterrows():
    p = influxdb_client.Point(new_measurement_name).field(field_name, row[field_column])
    for i,v in row.items():
      if i != "_time" and i != field_column:
        p.tag(i,v)
    timestamp = row["_time"]
    p.time(timestamp)
    write_api.write(bucket=bucket, org=org, record=p)
  return


def get_dataframe_from_fluxtable(fluxtable):
  df = pd.DataFrame()
  for table in fluxtable:  
    table_list = []
    for record in table.records:
      columns_list = []
      value_list = []

      for key in record.values:
          columns_list.append(key)
          value_list.append(record[key])

      table_list.append(value_list)
    df_table = pd.DataFrame(table_list, columns=columns_list)
    df = df._append(df_table, ignore_index=True)
  return df


def get_all_filenames_by_jobname(jobname, bucket="hsebucket", measurement="libiotrace"):
  query = f'''
    import "influxdata/influxdb/schema"

    schema.tagValues(
      bucket: "{bucket}",
      tag: "filename",
      predicate: (r) => r._measurement == "{measurement}" and r.jobname == "{jobname}",
      start: -{time_range},
    )
  '''
  filenames_fluxtable = query_api.query(org=org, query=query)
  filenames = get_dataframe_from_fluxtable(filenames_fluxtable)["_value"].unique()

  # MPI lock file handling
  filenames = [ file for file in filenames if ".locktest." not in file ]

  return filenames


def aggregate_timestamps(dataframe):
  ignore_fields_list = [
    #"file_type_stream",
    "function_data_file_name",
    #"function_data_id_device_id",
    #"function_data_id_inode_nr",
    "function_data_mode",
    #"function_name",
    #"hostname",
    #"pid",
    #"return_state",
    #"tid",
    #
    #"time_diff",
    "time_end",
    "time_start",
    #"traced_filename",
    "wrapper_time_end",
    "wrapper_time_start",

    #MPI
    #"function_data_type",
    #"function_data_size",
    #"function_data_id_inode_nr",
    #"function_data_id_device_id",
    #"function_data_blocksize",
    #"return_state",
  ]


  current_timestamp = ""
  fields = []
  values = []
  df_new = pd.DataFrame()
  dataframe = dataframe.sort_values("_time")
  for index, row in dataframe.iterrows():
    timestamp = row["_time"]
    field = row["_field"]
    value = row["_value"]

    if field not in ignore_fields_list:
      if timestamp == current_timestamp:
        fields.append(field)
        values.append(value)
      else:
        if current_timestamp != "":
          df_timestamp = pd.DataFrame([values], columns=fields)
          df_new = pd.concat([df_new,df_timestamp], ignore_index=True)
        fields = []
        values = []
        fields.append("_time")
        values.append(timestamp)
        fields.append(field)
        values.append(value)
      current_timestamp = timestamp
  df_new = df_new.sort_values("_time")

  # MPI Handling - remove non-MPI functions
  if "file_type_mpi_file" in df_new.columns:
    df_new = df_new[df_new["file_type_mpi_file"].notnull()]

  return df_new


def calculate_offsets(aggregated_dataframe, jobname):
  # MPI Handling
  streams = None
  # TODO: Add file_type_descriptor
  if "file_type_stream" in aggregated_dataframe.columns:
    streams = aggregated_dataframe["file_type_stream"].unique()
  elif "file_type_mpi_file" in aggregated_dataframe.columns:
    streams = aggregated_dataframe["file_type_mpi_file"].unique()
  else:
    print("!!!!! No possible Stream Handling detected !!!!!")
  hosts = aggregated_dataframe["hostname"].unique()
  processids = aggregated_dataframe["pid"].unique()

  df_offset = pd.DataFrame()
  for host in hosts:
    for processid in processids:
      for stream in streams:
        if "file_type_stream" in aggregated_dataframe.columns:
          streamdata = aggregated_dataframe[
                            (aggregated_dataframe["file_type_stream"]==stream) & 
                            (aggregated_dataframe["hostname"]==host) & 
                            (aggregated_dataframe["pid"]==processid)
                        ].copy()
        else:
          streamdata = aggregated_dataframe[
                            (aggregated_dataframe["hostname"]==host) & 
                            (aggregated_dataframe["pid"]==processid) & 
                            (aggregated_dataframe["file_type_mpi_file"]==stream)
                        ].copy()
          
        streamdata["offset_after"] = 0
        streamdata["access_count_id"] = ""
        streamdata["jobname"] = jobname
        streamdata["file_access_identifier"] = f"{host}_{processid}_{stream}"
        offset = 0
        access_count = 0
        function_data_id_device_id = ""
        function_data_id_inode_nr = ""

        if streamdata.shape[0] == 1 and streamdata.iloc[0].function_name in function_close_list:
          print("Stream with only close is skipped")
          break

        for index, row in streamdata.iterrows():
          func = row.function_name
          if func in function_open_list:
            offset = 0
            access_count += 1
            function_data_id_device_id = row.function_data_id_device_id
            function_data_id_inode_nr = row.function_data_id_inode_nr
            pass
          elif func in function_close_list:
            offset = 0
          elif func in function_write_list:
            offset += row.function_data_written_bytes
          elif func in function_read_list:
            offset += row.function_data_read_bytes
          elif func in function_seek_list or func in function_seek2_list:
            if row.function_data_relative_to == "beginning_of_file":
              offset = row.function_data_offset
            else:
              print(f"!!!!! Can't handle seek for {row.function_data_relative_to} yet !!!!!")
          else:
            print(f"!!!!! Function {func} can't be handled yet. !!!!!")
          streamdata.loc[index, "offset_after"] = offset
          streamdata.loc[index, "access_count_id"] = f"{host}_pid{processid}_stream{stream}_{row["traced_filename"]}_nr{access_count}"
          streamdata.loc[index, "function_data_id_device_id"] = function_data_id_device_id
          streamdata.loc[index, "function_data_id_inode_nr"] = function_data_id_inode_nr
        df_offset = df_offset._append(streamdata, ignore_index=False)

  df_offset = df_offset.sort_values("_time")
  return df_offset


def redefine_offset_timestamps(offset_df):
  global first_timestamp
  global last_timestamp
  offset_df = offset_df.sort_values("_time")
  current_mikro_ts = ""
  current_milli_ts = ""
  for index, row in offset_df.iterrows():
    mikro_ts = row["_time"]
    milli_ts = mikro_ts - timedelta(microseconds=mikro_ts.microsecond % 1000)
    if current_mikro_ts != "" and mikro_ts > current_mikro_ts and milli_ts <= current_milli_ts:
      milli_ts = current_milli_ts + timedelta(milliseconds=1)
      offset_df.loc[index, "_time"] = milli_ts

    current_mikro_ts = mikro_ts
    current_milli_ts = milli_ts
    if first_timestamp == "":
      first_timestamp = int(current_milli_ts.timestamp() * 1000)
  last_timestamp = int(current_milli_ts.timestamp() * 1000)
  return offset_df



def pipeline_by_jobname_and_filename(jobname, filename, new_measurement_name, bucket="hsebucket", measurement="libiotrace"):
  print(f"Querying data from Influx (Job: {jobname}, File: {filename})...")
  start = time.time()
  raw_result = get_fluxtable_by_jobname_and_filename(jobname, filename, bucket, measurement)
  end = time.time()
  print(f"Querying data done. ({round(end-start,2):.2f}s)")

  print("Processing data...")
  start = time.time()
  raw_df = get_dataframe_from_fluxtable(raw_result)

  if filename == "mptf_3.txt":
    print(raw_df.columns)
    raw_df.sort_values("_time")
    print(raw_df.sort_values("_time").loc[(raw_df["hostname"] == "Baghira") & 
                        (raw_df["filename"] == "mptf_3.txt"), 
                        ["_time", "_value", "_field", "filename", "functionname", "hostname", "processid", "thread"]].iloc[952:].head(40))

  combined_df = aggregate_timestamps(raw_df)

  # For debug, show example data sctructure
  """ if filename == "mptf_3.txt":
    print(combined_df.loc[(combined_df["file_type_stream"] == "0x7f0458005f90") & 
                        (combined_df["traced_filename"] == "mptf_3.txt"), 
                        ["_time", "traced_filename", "function_data_read_bytes", "function_name", "hostname", "pid", "tid", "file_type_stream"]].loc[70:].head(1))
  
  if filename == "mptf_3.txt":
    print(combined_df.columns)
    print(combined_df.loc[(combined_df["file_type_stream"] == "0x7f0458005f90") & 
                        (combined_df["traced_filename"] == "mptf_3.txt"), 
                        ["_time", "traced_filename", "function_data_id_inode_nr", "function_data_id_device_id", "function_name", "hostname", "pid", "tid", "file_type_stream"]].loc[69:].head(1)) """
  
  offset_df = calculate_offsets(combined_df, jobname)
  end = time.time()

  print(f"Processing data done. ({round(end-start,2):.2f}s)")
  start = time.time()
  retimed_df = redefine_offset_timestamps(offset_df)
  end = time.time()
  print(f"Renewing timestamps done. ({round(end-start,2):.2f}s)")
  
  if filename == "mptf_3.txt":
    print(retimed_df.columns)
    print(retimed_df.loc[(retimed_df["file_access_identifier"] == "Baghira_20571_0x7f0458005f90") & 
                        (retimed_df["traced_filename"] == "mptf_3.txt"), 
                        ["_time", "function_name", "function_data_written_bytes", "function_data_read_bytes", "offset_after"]].loc[69:])
  print("Renewing timestamps...")

  print("Writing data to Influx...")
  start = time.time()
  print(retimed_df)
  print(retimed_df.loc[retimed_df["pid"] == 10244])
  write_data_to_influx(retimed_df,new_measurement_name,bucket)
  end = time.time()
  print(f"Writing to Influx done. ({round(end-start,2):.2f}s)")

  print()
  extract_optimizations(retimed_df, new_measurement_name)


def batch_all_files_of_jobname(jobname, new_measurement_name, bucket="hsebucket", measurement="libiotrace"):
  processing_start = time.time()
  print(f"||| Batch processing of all files used by job {jobname} started |||")
  print("----------------------")
  except_filenames = [
    "_ NOT FOUND _",
    "_ NOT-A-FILE _",
    "_ STD-IO _",
    "_ PSEUDO-FILE _",
    "__ UNKNOWN (NOT-YET-SUPPORTED) __",
  ]
  all_files = get_all_filenames_by_jobname(jobname, bucket, measurement)
  for filename in all_files:
    if filename not in except_filenames and "/" not in filename :
      print(filename)
      file_processing_start = time.time()
      pipeline_by_jobname_and_filename(jobname, filename, new_measurement_name, bucket, measurement)
      file_processing_end = time.time()
      file_processing_duration = round(file_processing_end - file_processing_start, 2)
      print(f"Processing of data for {filename} done in {file_processing_duration:.2f}s")
      print("----------------------")
  processing_end = time.time()
  processing_duration = round(processing_end-processing_start, 2)
  print(f"||| Batch processing done in {processing_duration:.2f}s, data available in measurement {new_measurement_name} in bucket {bucket} |||")
  print(f"Dashboard Link ----> {generate_dashboard_url(jobname, new_measurement_name, bucket)}")


def generate_dashboard_url(jobname, new_measurement_name, bucket="hsebucket"):
  dashboard_id = "y6u33e4-cd6f-4bc8-a718-01fa8ec8d26a"
  dashboard_name = "file-access-analysis"
  dashboard_url = f"http://localhost:3000/d/{dashboard_id}/{dashboard_name}?orgId=1&from={first_timestamp}"\
    f"&to={last_timestamp}&var-bucket={bucket}&var-measurement={new_measurement_name}&var-jobname={jobname}"
  return dashboard_url



def check_if_can_be_optimized(repeated_function, byte_range_start, byte_range_end, call_between):
  print(f"### Repeated: {repeated_function} from {byte_range_start} to {byte_range_end}")
  
  between_func = call_between["function_name"]
  between_range_start = 0
  if between_func in function_read_list:
    between_range_start = call_between["offset_after"] - call_between["function_data_read_bytes"]
  elif between_func in function_write_list:
    between_range_start = call_between["offset_after"] - call_between["function_data_written_bytes"]
  elif between_func in function_seek_list:
    between_range_start = call_between["offset_after"]
  else:
    print(f"%%%%%%%% CANNOT HANDLE BETWEEN function {between_func}!! %%%%%%%%%")
  between_range_end = call_between["offset_after"]
  print(f"### Between: {between_func} from {between_range_start} to {between_range_end}")
  print("# CHECKING IF CAN BE OPTIMIZED....")
  # categories: 0 - not optimizable, 2 - call between but optimizable, 3 - function calls unclear
  category = 0
  info = "This is info about the potential optimization (placeholder)"
  if repeated_function in function_read_list:
    # Read as repeated function can be optimized if
    if between_func in function_read_list:
      # Read is between
      category = 2
      info = "Can be optimized: read doesn't interfere with reads"
    elif between_func in function_write_list:
      if byte_range_end < between_range_start:
        # Write is behind reads
        category = 2
        info = "Can be optimized: write happens behind reads"
      else:
        info = "Cannot be optimized: write in or in front of area of reads"
    elif between_func in function_seek_list:
      # Seek is between
      category = 2
      info = "Can be optimized: seek doesn't interfere with reads"
    else:
      # Function between can't be handled
      category = 3
      info = f"Optimization unclear: interfering function {between_func} cannot be handled"
  elif repeated_function in function_write_list:
    # Write as repeated function can be optimized if
    if between_func in function_read_list:
      if between_range_end < byte_range_start:
        # Read is before writes
        category = 2
        info = "Can be optimized: read happens in front of writes"
      else:
        info = "Cannot be optimized: read in or behind area of writes"
    elif between_func in function_seek_list:
      if between_range_end < byte_range_start:
        # Seek is before writes
        category = 2
        info = "Can be optimized: seek happens in front of writes"
      else:
        info = "Cannot be optimized: seek in or behind area of writes"
    elif between_func in function_write_list:
      info = "Cannot be optimized: write anywhere between writes leads to problems"
    else:
      # Function between can't be handled
      category = 3
      info = f"Optimization unclear: interfering function {between_func} cannot be handled"
  else:
    # Repeated function can't be handled
    category = 3
    info = f"Optimization unclear: repeated function {repeated_function} cannot be handled"

  return category, info


def extract_optimizations(df, new_measurement_name):
  print("--------------")
  print("Calculating possible optimizations...")
  faids_list = df["file_access_identifier"].unique()
  print(faids_list)
  optimizations_dict = {}
  columns_list = ["_time","opt_id","jobname","traced_filename","function_data_id_device_id","function_data_id_inode_nr","file_access_identifier","function","category"]
  optimizations_df = pd.DataFrame(columns=columns_list)
  for faid in faids_list:
    df_faid = df[df["file_access_identifier"] == faid].sort_values("_time")
    timestamp_list = []
    previous_function = ""
    previous_timestamp = ""
    opt_count = 0
    previous_row = pd.DataFrame()
    multiple_repeats = False
    for index, row in df_faid.iterrows():
      current_function = row["function_name"]
      current_timestamp = row["_time"]
      
      if current_function == previous_function:
        if not multiple_repeats:
          opt_count += 1
        optid = f"{faid}_{opt_count}"
        point_info = [current_timestamp,optid,row["jobname"],row["traced_filename"],row["function_data_id_device_id"], row["function_data_id_inode_nr"], row["file_access_identifier"],current_function,0]
        point_df = pd.DataFrame([point_info], columns=columns_list)
        optimizations_df = pd.concat([optimizations_df, point_df], ignore_index=True)
        if timestamp_list == []:
          timestamp_list.append(previous_timestamp)
        timestamp_list.append(current_timestamp)

        multiple_repeats = True

        if optid not in optimizations_dict:
          optimizations_dict[optid] = [previous_row]
        optimizations_dict[optid].append(row)

      else:
        timestamp_list = []
        multiple_repeats = False
          
      previous_function = current_function
      previous_timestamp = current_timestamp
      previous_row = row

    print(f"############# ENDE FAID {faid}")
  

  opt_columns_list = ["_time","opt_id","jobname","traced_filename","function_data_id_device_id","function_data_id_inode_nr","file_access_identifier","call_1_time","call_2_time","function","category"]
  analyzed_optimizations = pd.DataFrame(columns=opt_columns_list)
  for oid,pointlist in optimizations_dict.items():
    current_faid = pointlist[0]["file_access_identifier"]
    current_func = pointlist[0]["function_name"]
    first_call_time = pointlist[0]["_time"]
    last_call_time = pointlist[-1]["_time"]
    reduced_df = df[(df["file_access_identifier"] != current_faid) & (df["_time"] > first_call_time) & (df["_time"] < last_call_time) & (~df["function_name"].isin(function_open_list)) & (~df["function_name"].isin(function_close_list))]
    for index, point in enumerate(pointlist):
      if index+1 >= len(pointlist):
        # last item in list
        break
      next_point = pointlist[index+1]
      start_time = point["_time"]
      end_time = next_point["_time"]

      # Categories: 0 - can't be optimized, 1 - can be easily optimized, 2 - something between but can be optimized
      opt_category = 0
      info_string = ""

      calls_between_df = reduced_df[(reduced_df["_time"] > start_time) & (reduced_df["_time"] < end_time)]
      if calls_between_df.empty:
        # No function call of any other faid
        opt_category = 1
        info_string = "Can be optimized: no calls between"

      else:
        print("+++++++++ Analyzing needed... ++++++++++")
        
        repeated_function = point["function_name"]
        byte_range_start = 0
        if repeated_function in function_read_list:
          byte_range_start = point["offset_after"] - point["function_data_read_bytes"]
        elif repeated_function in function_write_list:
          byte_range_start = point["offset_after"] - point["function_data_written_bytes"]
        else:
          print(f"%%%%%%%% CANNOT HANDLE function {repeated_function}!! %%%%%%%%%")
        byte_range_end = next_point["offset_after"]

        for index, call_between in calls_between_df.iterrows():
          opt_category, info_string = check_if_can_be_optimized(repeated_function, byte_range_start, byte_range_end, call_between)
          # If any call between makes a problem, break loop
          if opt_category == 0 or opt_category == 3:
            print(f"!!!! {info_string}")
            break

      singlePointDF = createOptimizationPoint(oid, point, next_point, opt_category, info_string)
      analyzed_optimizations = pd.concat([analyzed_optimizations, singlePointDF], ignore_index=True)
      pass

  #write_data_to_influx(optimizations_df,new_measurement_name,field_name="category",field_column="category")
  write_data_to_influx(analyzed_optimizations,new_measurement_name,field_name="category",field_column="cat_val")
  return

def createOptimizationPoint(oid, point_one, point_two, category, info):
  opt_columns_list = ["_time","opt_id","jobname","hostname","traced_filename","function_data_id_device_id","function_data_id_inode_nr","file_access_identifier","call_1_time","call_2_time","function","category","info","cat_val"]
  call_1_time = point_one["_time"]
  call_2_time = point_two["_time"]
  mean_time = call_1_time + (call_2_time - call_1_time) / 2
  opt_info_list = [mean_time,oid,point_one["jobname"],point_one["hostname"],point_one["traced_filename"],point_one["function_data_id_device_id"], point_one["function_data_id_inode_nr"], point_one["file_access_identifier"], call_1_time, call_2_time, point_one["function_name"], category, info, category]
  single_opt_df = pd.DataFrame([opt_info_list], columns=opt_columns_list)
  return single_opt_df

# MPI call:
#batch_all_files_of_jobname("mpi_file_io_1k_updated", "cursor_mpi_test_10_21_test3", bucket="mpi_test")

#Working Posix call:
batch_all_files_of_jobname("multi_shared2024-04-04_14-00-17_iotrace.log", "11_18_finished")

# Call structure:
# batch_all_files_of_jobname("desired-libiotrace-jobname.log", "any-new-measurement-name")
