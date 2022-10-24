/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_LOGGING_H
#define STEP_2_LOGGING_H

#include <iostream>

// im only going to make this class if I have time (to add dates, output to file, and colored severity)

// log fatal
#define flog std::cerr << "[Fatal]: "
// log error
#define elog std::cerr << "[Error]: "
// log warning
#define wlog std::cout << "[Warning]: "
// log info
#define ilog std::cout << "[Info]: "
// log debug
#define dlog std::cout << "[Debug]: "
// log trace
#define tlog std::cout << "[Trace]: "


#endif //STEP_2_LOGGING_H
