#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <bitset>
#include <chrono> // for std::chrono functions

#include "ltc2495.h"
using namespace std;
class Timer
{
private:
	// Type aliases to make accessing nested type easier
	using Clock = std::chrono::steady_clock;
	using Second = std::chrono::duration<double, std::ratio<1> >;

	std::chrono::time_point<Clock> m_beg { Clock::now() };

public:
	void reset()
	{
		m_beg = Clock::now();
	}

	double elapsed() const
	{
		return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
	}
};

int main() {
    const uint8_t deviceAddress = 0x45;   // Specify the LTC2495 device address
    bool b[16]={true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};
    uint8_t g[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    uint8_t c = 0;
    /*
    usleep(1000);
    adc a = adc(deviceAddress);
    a.addChannel(c,1);
    usleep(100);

    cout << a.readOne(c)<<endl;
    usleep(100);
    
    cout << a.readOne(c)<<endl;
    a.endi2c();
    usleep(75000);
    c=5;
    a.i2cstart(deviceAddress);
    a.addChannel(c,1);
    usleep(100);

    cout << a.readOne(c)<<endl;
    usleep(100);
    
    cout << a.readOne(c)<<endl;
    a.endi2c();
    usleep(10000);

    */
    //cout << "Trying ALL" << endl;
    Timer t;
    adc a2 = adc(b,g,deviceAddress);
    float t1 = t.elapsed();
    a2.readAll();
    while (true)
    {
    a2.readOne(0);
    cout << a2.res[0]<<endl;        
    /*for(int i = 0; i<16; i++){
        if (b[i]){
        cout << a2.res[i]<<endl;
        }
    }*/

    }
    float t2 = t.elapsed();
    //cout <<"Timer now is "<<(t2-t1)/16<<endl;
    return 0;
}


