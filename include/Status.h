#pragma once

enum Status : unsigned char {
    Ok = 0,
    NotFound = 1,
    Corruption = 2,
    NotSupported = 3,
    InvalidArgument = 4,
    IOError = 5,
    OutOfMemory = 6,
    Unknown = 255
};