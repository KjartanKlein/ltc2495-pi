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
using namespace std;


class channel{
    public:
    int i2c;
    uint8_t nr;
    uint8_t gain;
    bool speed;
    bool sngl;
    uint8_t reject_mode;
    bool temp;
    float refvoltage;
    float res;

    channel(uint8_t _nr, uint8_t _gain, bool _speed, bool _sngl, bool _reject_mode, bool _temp, float _refvolt, int _i2c);
    channel(uint8_t _nr, uint8_t _gain, int _i2c);
    channel();
    int send_config();
    int send_last();
    float read_ch();
    uint16_t makeConfig();
    void HRconfig();
    private:
        bool ODD;
        uint8_t HB;
        uint8_t LB;
        uint8_t TB;
        uint8_t configLast;
        uint8_t G;
};
channel::channel(){
    i2c=-1;
    nr=255;
    gain=255;
    speed=false;
    sngl=false;
    reject_mode=false;
    temp=false;
    refvoltage=0.0;
    res= 0.0;


}
channel::channel(uint8_t _nr, uint8_t _gain, bool _speed, bool _sngl, bool _reject_mode, bool _temp, float _refvolt, int _i2c){
    nr = _nr;
    gain = _gain;
    speed = _speed;
    sngl =_sngl;
    reject_mode =_reject_mode;
    temp=_temp;
    refvoltage=_refvolt;
    i2c = _i2c;
    if(nr > 16){
        nr = 16;
    }
    if(nr < 0){
        nr = 0;
    }
    switch (gain){
        case 1:
        G=0;
        break;
        case 2:
        G=1;
        break;
        case 4:
        G=2;
        break;
        case 8:
        G=3;
        break;
        case 16:
        G=4;
        break;
        case 32:
        G=5;
        break;
        case 64:
        G=6;
        break;
        case 128:
        G=7;
        break;
    } 
    makeConfig();

}
channel::channel(uint8_t _nr, uint8_t _gain, int _i2c){
    nr = _nr;
    gain = _gain;
    speed = true;
    sngl = true;
    reject_mode =0;
    temp= false;
    refvoltage= 3.3;
    i2c=_i2c;
    if (nr%2 ==0){
            ODD=false;
        }else{
            ODD=true;
    }
    if(nr > 16){
        nr = 16;
    }
    if(nr < 0){
        nr = 0;
    }
    switch (gain){
        case 1:
        G=0;
        break;
        case 2:
        G=1;
        break;
        case 4:
        G=2;
        break;
        case 8:
        G=3;
        break;
        case 16:
        G=4;
        break;
        case 32:
        G=5;
        break;
        case 64:
        G=6;
        break;
        case 128:
        G=7;
        break;
    } 
    makeConfig();
    
}
uint16_t channel::makeConfig(){
    int n =0;
    if(ODD){
            //n--;
        }
    n = n + nr/2;

   // cout <<"Make config started on "<< nr <<endl;
    uint8_t nout= 0x00|((ODD<<3) & 0XFF)|(n & 0XFF);
    nout |= 1 << 4; 
    //std::bitset<8> y(nout);
    //cout <<"n set to " << n <<" nout set to "<<y<< "  ";
    HB=(0b10100000) | (nout & 0xff);
    LB=(0b10000000)| (temp<<6) |(reject_mode << 4) | (speed << 3) | (G);
    configLast =( HB >> 7) | LB;
    //HRconfig();
    return configLast;

}
void channel::HRconfig(){

    std::bitset<8> y(HB);
    std::bitset<8> z(LB);
    cout<<"config set to: "<<y<<"  "<<z<<endl;

}



//10 EN SGL ODD A2 A1 A0 EN2 IM FA FB SPD GS1 GS1

int channel::send_config(){
    uint8_t b[2];
    b[1] = LB;
    b[0] = HB;
    if (write(i2c, b, 2) != 2) {
        perror("Failed to set config");
        return -1;
    }else{
        return 0;
    }
}

int channel::send_last(){
    uint8_t b[2];
    //HRconfig();
    b[1] = LB & 0b110111111;
    b[0] = HB & 0b011111111;
    if (write(i2c, b, 2) != 2) {
        //perror("Failed to send read"); // will always throw error, should be read nak but dont aghve that tool
        return -1;
    }else{
        return 0;
    }
}

float channel::read_ch(){
    send_last();
    int i = 0;
    uint8_t readBuffer[3];
    while(true){
    if (read(i2c, readBuffer, sizeof(readBuffer)) != sizeof(readBuffer)) {
            //perror("Failed to read data");
            
        }else{
           // cout << "sucess "<< i;
            break;
        }
        usleep(100);
        i++;
        if(i==1000){
            cout<< "failure in read i = 1000";
            return -1;
        }
    }

    // Convert the received bytes to a 16-bit integer value
    uint16_t result = (static_cast<uint16_t>((readBuffer[0])&0X7F) << 8)  | static_cast<uint16_t>(readBuffer[1]);
    res = 2*refvoltage*(result)/(65536*gain);
    return res;
}


class adc{
    public:
    bool active_ch[16];
    channel ch[16];
    int gains[16];
    float res[16];
    float temp;
    int i2cF;
    uint8_t devAddress;
    adc(uint8_t);
    adc(bool[16], uint8_t[16], uint8_t);
    void addChannel(uint8_t, uint8_t);
    void readAll();
    float readOne(uint8_t);
    int i2cstart(uint8_t);
    void endi2c();

};

float adc::readOne(uint8_t i){
    res[i] = ch[i].read_ch();
    return res[i];
    
}

void adc::addChannel(uint8_t i, uint8_t gain){
    gains[i]=gain;
    active_ch[i]=true;
    ch[i]=channel(i,gains[i],i2cF);
    ch[i].send_config();
}

void adc::readAll(){
    for(int i=0; i<16; i++){
        if(active_ch[i]){
            //cout << i <<endl;
            readOne(i);

            usleep(75000);//needed for some reason
        }
    }
}

adc::adc(uint8_t devAdd){
    devAddress=devAdd;
    int i2cFile = i2cstart(devAddress);
    if(i2cFile == -1){
        perror("Bad file");
    }
    
    for(int i; i<16;i++){
        active_ch[i] = false;
        gains[i] = 1;
    }
}
adc::adc(bool b[16],uint8_t gain[16], uint8_t devAdd){
    devAddress=devAdd;
    int i2cFile = i2cstart(devAddress);
    if(i2cFile == -1){
        perror("Bad file");
    }
    for(int i=0; i<16;i++){
        //cout<<i<<endl;;
        active_ch[i] = b[i];
        gains[i] = gain[i];
        if(b[i]){
            ch[i]=channel(i,gains[i],i2cF);
            ch[i].send_config();
        }
        usleep(100000); // needed for i2c to work, ltc2495 sleeps alot
    }

}
int adc::i2cstart(uint8_t deviceAddress){
    const char* i2cDevice = "/dev/i2c-1";  // Specify the I2C device
   // Open the I2C device
    int i2cFile = open(i2cDevice, O_RDWR);
    if (i2cFile < 0) {
        perror("Failed to open I2C device");
        return -1;
    }

    // Set the LTC2495 device address
    if (ioctl(i2cFile, I2C_SLAVE, deviceAddress) < 0) {
        perror("Failed to set LTC2495 device address");
        close(i2cFile);
        return -1;
    }
    i2cF=i2cFile;
    return i2cFile;
}
void adc::endi2c(){
    close(i2cF);
}