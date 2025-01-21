//
// Created by yuly on 06.04.23.
//

#include "vector/DateColumnVector.h"
#include <algorithm>
#include <ctime>
DateColumnVector::DateColumnVector(uint64_t len, bool encoding): ColumnVector(len, encoding) {
	posix_memalign(reinterpret_cast<void **>(&dates), 32,len * sizeof(int32_t));
	memoryUsage += (long) sizeof(int) * len;
}

void DateColumnVector::close() {
	if(!closed) {
		if(/*encoding && */dates != nullptr) {
			free(dates);
		}
		dates = nullptr;
		ColumnVector::close();
	}
}

void DateColumnVector::print(int rowCount) {
	for(int i = 0; i < rowCount; i++) {
		std::cout<<dates[i]<<std::endl;
	}
}

DateColumnVector::~DateColumnVector() {
	if(!closed) {
		DateColumnVector::close();
	}
}

/**
     * Set a row from a value, which is the days from 1970-1-1 UTC.
     * We assume the entry has already been isRepeated adjusted.
     *
     * @param elementNum
     * @param days
 */
void DateColumnVector::set(int elementNum, int days) {
	if(elementNum >= writeIndex) {
		writeIndex = elementNum + 1;
	}
	dates[elementNum] = days;
	// TODO: isNull
}

void * DateColumnVector::current() {
    if(dates == nullptr) {
        return nullptr;
    } else {
        return dates + readIndex;
    }
}

void DateColumnVector::add(std::string &value) {
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);

	struct std::tm tm = {};  
	if (strptime(value.c_str(), "%Y-%m-%d", &tm)) {
		std::time_t time = std::mktime(&tm) +8*3600; //东八区
		long days = time / (24 * 60 * 60) ;
		add(days);
	} else {
		std::cerr << "Failed to parse date string: " << value << std::endl;
	}
}

void DateColumnVector::add(bool value) {
	add(value ? 1 : 0);
}

void DateColumnVector::add(int64_t value) {
    add(static_cast<int>(value));  
}

void DateColumnVector::add(int value) {
    if (writeIndex >= length) {
        ensureSize(writeIndex * 2, true);  
    }
    int index = writeIndex++;
    dates[index] = value;
    isNull[index] = false;
}

void DateColumnVector::ensureSize(uint64_t size, bool preserveData) {
    ColumnVector::ensureSize(size, preserveData);
    if (length < size) {
            int *oldVector = dates;
            posix_memalign(reinterpret_cast<void **>(&dates), 32,
                           size * sizeof(int32_t));
            if (preserveData) {
                std::copy(oldVector, oldVector + length, dates);
            }
            delete[] oldVector;
            memoryUsage += (long) sizeof(int) * (size - length);
            resize(size);
    }
}