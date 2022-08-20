#include <iostream>

#include "logging/InfluxDbWriter.h"
#include "logging/influxDb.h"
#include "utility/Debug.h"

InfluxDbWriter::InfluxDbWriter(std::string hostname, unsigned port, std::string dbName, std::size_t bufferSize) : buffer_size(bufferSize) {
    host = hostname;
    db = dbName;
    portStr = std::to_string(port);
}

void InfluxDbWriter::Init() {
    buffer = new char*[buffer_amount];
    for (size_t i = 0; i < buffer_amount; i++)
    {
        buffer[i] = new char[buffer_size];
    }

    if(initDbContext(&cntxt, host.c_str(), portStr.c_str(), db.c_str()) < 0) {
        throw new std::runtime_error("Couldn't initialize influxDbWriter (bad context)");
    }
}

InfluxDbWriter::~InfluxDbWriter() { 
    push();
    joinThreads();

    if(buffer != nullptr) {
        for (size_t i = 0; i < buffer_amount; i++)
        {
            delete buffer[i];
        }
        delete buffer;
    }
    
    (void) deInitDbContext(&cntxt);
}

void InfluxDbWriter::setCredentials(std::string user, std::string password) {
    std::string user_http = "&u=" + user;
    std::string pw_http = "&p=" + password;
    set_credentials(&cntxt, user_http.c_str(), pw_http.c_str());
}

void InfluxDbWriter::setTimestampPrecision(timestamp_precision_t precision) {
    set_timestamp_precision(&cntxt, precision);
}

void InfluxDbWriter::setMeasurement(std::string measurement) {
    this->measurement = measurement;
}

void InfluxDbWriter::startDataPoint() {
    if((buffer_size - buffer_pos) < (measurement.length())) {
        push();
    }
    
    buffer_pos += sprintf(&buffer[buffer_sel][buffer_pos], "%s", this->measurement.c_str());
}

void InfluxDbWriter::addTag(std::string key, std::string value) {
    if((buffer_size - buffer_pos) < (key.length() + value.length() + 2)) {
        push();
    }
    buffer_pos += sprintf(&buffer[buffer_sel][buffer_pos], ",%s=%s", key.c_str(), value.c_str());
}

void InfluxDbWriter::tagsDone() {
    if((buffer_size - buffer_pos) < 1) {
        push();
    }
    buffer[buffer_sel][buffer_pos] = ' ';
    buffer_pos++;
}

// Properly sanitize strings if needed, neglected so far because of the overhead (DB)
void InfluxDbWriter::addField(std::string key, std::string value) {
    if((buffer_size - buffer_pos) < (key.length() + value.length() + 4)) {
        push();
    }
    buffer_pos += sprintf(&buffer[buffer_sel][buffer_pos], "%s=\"%s\",", key.c_str(), value.c_str());
}


void InfluxDbWriter::addField(std::string key, std::size_t value) {
    std::string str = std::to_string(value);
    if((buffer_size - buffer_pos) < (key.length() + str.length() + 3)) {
        push();
    }
    buffer_pos += sprintf(&buffer[buffer_sel][buffer_pos], "%s=%si,", key.c_str(), str.c_str());
}

// Might let user select the precision and scientific notation
void InfluxDbWriter::addField(std::string key, double value) {
    std::string str = std::to_string(value);

    if((buffer_size - buffer_pos) < (key.length() + str.length() + 2)) {
        push();
    }
    buffer_pos += sprintf(&buffer[buffer_sel][buffer_pos], "%s=%s,", key.c_str(), str.c_str());
}

void InfluxDbWriter::addField(std::string key, bool value) {
    char c = value ? 't' : 'f';

    if((buffer_size - buffer_pos) < (key.length() + 2)) {
        push();
    }
    buffer_pos += sprintf(&buffer[buffer_sel][buffer_pos], "%s=%c,", key.c_str(), c);
}

void InfluxDbWriter::endDataPoint(std::size_t timestamp) {
    std::string ts_str = std::to_string(timestamp);
    
    if((buffer_size - buffer_pos) < (ts_str.length() + 1)) {
        push();
    }
    buffer[buffer_sel][buffer_pos-1] = ' ';
    buffer_pos += sprintf(&buffer[buffer_sel][buffer_pos], "%s\n", ts_str.c_str());
    last_measurement = buffer_pos-1;
}

void InfluxDbWriter::push() { 
    // Might throw an exception if last measurement = 0 as this equals a too long entry (DB)
    if (last_measurement > 0) {
        joinThreads();
        threads.push_back(std::thread(sendData, &cntxt, this->buffer[buffer_sel],  last_measurement));
        transferPartialWrite();
        buffer_sel = (buffer_sel + 1) & 0b1;
    }
}

void InfluxDbWriter::transferPartialWrite() {
    std::size_t i = last_measurement;
    
    while(i < buffer_pos)
    {
        buffer[(buffer_sel + 1) & 0b1][i - last_measurement] = buffer[buffer_sel][i];   
        i++;
    }

    buffer_pos = i - last_measurement;
    last_measurement = 0;
}

void InfluxDbWriter::joinThreads() {
    for (std::thread &t : threads) {
        if(t.joinable()) t.join();
//        else Debug::warning("InfluxDbWriter thread was not joinable."); // causes crash during "Waiting for nodes..."
    }
} 

void InfluxDbWriter::flush() {
    push();
}
