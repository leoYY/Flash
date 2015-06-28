// Copyriht (c) 2015, Flash Authors. All Rights Reserved
// Author: yuanyi03@baidu.com
#ifndef PS_SPI_NONCOPYABLE_H
#define PS_SPI_NONCOPYABLE_H

namespace common {
namespace util {

class noncopyable {
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    noncopyable(const noncopyable&);
    noncopyable& operator=(const noncopyable&);
};

}   // ending namespace util
}   // ending namespace common

#endif  //PS_SPI_NONCOPYABLE_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
