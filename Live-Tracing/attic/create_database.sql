create database if not exists grafana_iotrace;

create table if not exists grafana_iotrace.collections(
    id binary(16) not null unique,
    name varchar(255) not null,
    constraint pk_collection primary key (id)
);

create table if not exists grafana_iotrace.hosts(
    collection_id binary(16) not null,
    name varchar(255) not null,
    constraint pk_host primary key (collection_id, name),
    constraint fk_collection_host foreign key (collection_id) references grafana_iotrace.collections(id)
);

create table if not exists grafana_iotrace.processes(
    collection_id binary(16) not null,
    id int not null,
    host_name varchar(255) not null,
    constraint pk_process primary key (collection_id, id),
    constraint fk_host_process foreign key (collection_id, host_name) references grafana_iotrace.hosts(collection_id, name)
);

create table if not exists grafana_iotrace.threads(
    collection_id binary(16) not null,
    id int not null,
    pid int not null,
    constraint pk_thread primary key (collection_id, id),
    constraint fk_process_thread foreign key (collection_id, pid) references grafana_iotrace.processes(collection_id, id)
);

create table if not exists grafana_iotrace.files(
    collection_id binary(16) not null,
    file varchar(255) not null,
    kind varchar(40) not null,
    constraint pk_file primary key (collection_id, file),
    constraint fk_collection_file foreign key (collection_id) references grafana_iotrace.collections(id)
);

create table if not exists grafana_iotrace.file_names(
    collection_id binary(16) not null,
    file varchar(255) not null,
    name varchar(255) not null,
    constraint pk_file primary key (collection_id, file, name),
    constraint fk_file_file_name foreign key (collection_id, file) references grafana_iotrace.files(collection_id, file)
);

create table if not exists grafana_iotrace.functions(
    collection_id binary(16) not null,
    id int not null,
    name varchar(255) not null,
    type varchar(20),
    error bit not null,
    time long not null,
    wrapper_time long,
    bytes long,
    start_time long not null,
    end_time long not null,
    thread_id int not null,
    constraint pk_function primary key (collection_id, id),
    constraint fk_thread_function foreign key (collection_id, thread_id) references grafana_iotrace.threads(collection_id, id)
);

create table if not exists grafana_iotrace.function_manipulates_file(
    collection_id binary(16) not null,
    function_id int not null,
    file varchar(255) not null,
    constraint pk_function_manipulates_file primary key (collection_id, function_id, file),
    constraint fk_function foreign key (collection_id, function_id) references grafana_iotrace.functions(collection_id, id),
    constraint fk_file foreign key (collection_id, file) references grafana_iotrace.files(collection_id, file)
);

-- create table if not exists grafana_iotrace.overlapping_function_calls(
--     collection_id binary(16) not null,
--     id1 int not null,
--     id2 int not null,
--     constraint pk_overlapping_function_calls primary key (collection_id, id1, id2),
--     constraint fk_overlapping_function_calls_function1 foreign key (collection_id, id1) references grafana_iotrace.functions(collection_id, id),
--     constraint fk_overlapping_function_calls_function2 foreign key (collection_id, id2) references grafana_iotrace.functions(collection_id, id),
--     constraint fk_collection_overlapping_function_calls foreign key (collection_id) references grafana_iotrace.collections(id)
-- );

-- create table if not exists grafana_iotrace.overlapping_file_range(
--     collection_id binary(16) not null,
--     id1 int not null,
--     id2 int not null,
--     constraint pk_overlapping_file_range primary key (collection_id, id1, id2),
--     constraint fk_overlapping_file_range_function1 foreign key (collection_id, id1) references grafana_iotrace.functions(collection_id, id),
--     constraint fk_overlapping_file_range_function2 foreign key (collection_id, id2) references grafana_iotrace.functions(collection_id, id),
--     constraint fk_collection_overlapping_file_range foreign key (collection_id) references grafana_iotrace.collections(id)
-- );

create user if not exists 'grafana'@'localhost' identified by 'it-s-a-test';
grant select on grafana_iotrace.collections to 'grafana'@'localhost';
grant select on grafana_iotrace.hosts to 'grafana'@'localhost';
grant select on grafana_iotrace.processes to 'grafana'@'localhost';
grant select on grafana_iotrace.threads to 'grafana'@'localhost';
grant select on grafana_iotrace.files to 'grafana'@'localhost';
grant select on grafana_iotrace.file_names to 'grafana'@'localhost';
grant select on grafana_iotrace.functions to 'grafana'@'localhost';
grant select on grafana_iotrace.function_manipulates_file to 'grafana'@'localhost';
