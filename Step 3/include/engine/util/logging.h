/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_LOGGING_H
#define STEP_2_LOGGING_H

#include <iostream>

// im only going to make this class if I have time (to add dates, output to file, and colored severity)

// log fatal
#define flog std::cerr << "[Fatal]:\t"
// log error
#define elog std::cerr << "[Error]:\t"
// log warning
#define wlog std::cout << "[Warning]:\t"
// log info
#define ilog std::cout << "[Info]: \t"
// log debug
#define dlog std::cout << "[Debug]:\t"
// log trace
#define tlog std::cout << "[Trace]:\t"


#endif //STEP_2_LOGGING_H
