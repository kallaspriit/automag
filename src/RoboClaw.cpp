#include "RoboClaw.hpp"
#include <stdarg.h>

#define MAXTRY 1
#define SetDWORDval(arg) (uint8_t)(arg >> 24), (uint8_t)(arg >> 16), (uint8_t)(arg >> 8), (uint8_t)arg
#define SetWORDval(arg) (uint8_t)(arg >> 8), (uint8_t)arg

RoboClaw::RoboClaw(uint8_t adr, int baudrate, PinName rx, PinName tx) : _roboclaw(tx, rx)
{
  _roboclaw.baud(baudrate);
  address = adr;
}

void RoboClaw::crc_clear()
{
  crc = 0;
}

void RoboClaw::crc_update(uint8_t data)
{
  int i;
  crc = crc ^ ((uint16_t)data << 8);
  for (i = 0; i < 8; i++)
  {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
}

uint16_t RoboClaw::crc_get()
{
  return crc;
}

void RoboClaw::write_n(uint8_t cnt, ...)
{
  //uint8_t retry = MAXTRY;
  //do {
  crc_clear();
  va_list marker;
  va_start(marker, cnt);
  for (uint8_t index = 0; index < cnt; index++)
  {
    uint8_t data = va_arg(marker, unsigned int);
    crc_update(data);
    _roboclaw.putc(data);
  }
  va_end(marker);
  uint16_t crc = crc_get();
  _roboclaw.putc(crc >> 8);
  _roboclaw.putc(crc);
  //} while(_roboclaw.getc() != 0xFF);
}

void RoboClaw::write_(uint8_t command, uint8_t data, bool reading, bool crcon)
{
  _roboclaw.putc(address);
  _roboclaw.putc(command);

  if (reading == false)
  {
    if (crcon == true)
    {
      uint8_t packet[2] = {address, command};
      uint16_t checksum = crc16(packet, 2);
      _roboclaw.putc(checksum >> 8);
      _roboclaw.putc(checksum);
    }
    else
    {
      uint8_t packet[3] = {address, command, data};
      uint16_t checksum = crc16(packet, 3);
      _roboclaw.putc(data);
      _roboclaw.putc(checksum >> 8);
      _roboclaw.putc(checksum);
    }
  }
}

uint16_t RoboClaw::crc16(uint8_t *packet, int nBytes)
{
  uint16_t crc_ = 0;
  for (int byte = 0; byte < nBytes; byte++)
  {
    crc_ = crc_ ^ ((uint16_t)packet[byte] << 8);
    for (uint8_t bit = 0; bit < 8; bit++)
    {
      if (crc_ & 0x8000)
      {
        crc_ = (crc_ << 1) ^ 0x1021;
      }
      else
      {
        crc_ = crc_ << 1;
      }
    }
  }
  return crc_;
}

uint8_t RoboClaw::read_(void)
{
  return (_roboclaw.getc());
}

void RoboClaw::ForwardM1(int speed)
{
  write_(M1FORWARD, speed, false, false);
}

void RoboClaw::BackwardM1(int speed)
{
  write_(M1BACKWARD, speed, false, false);
}

void RoboClaw::ForwardM2(int speed)
{
  write_(M2FORWARD, speed, false, false);
}

void RoboClaw::BackwardM2(int speed)
{
  write_(M2BACKWARD, speed, false, false);
}

void RoboClaw::Forward(int speed)
{
  write_(MIXEDFORWARD, speed, false, false);
}

void RoboClaw::Backward(int speed)
{
  write_(MIXEDBACKWARD, speed, false, false);
}

void RoboClaw::ReadFirm()
{
  write_(GETVERSION, 0x00, true, false);
}

// int32_t RoboClaw::ReadEncM1()
// {
//   uint16_t read_byte[7];
//   write_n(2, address, GETM1ENC);

//   read_byte[0] = (uint16_t)_roboclaw.getc();
//   read_byte[1] = (uint16_t)_roboclaw.getc();
//   read_byte[2] = (uint16_t)_roboclaw.getc();
//   read_byte[3] = (uint16_t)_roboclaw.getc();
//   read_byte[4] = (uint16_t)_roboclaw.getc();
//   read_byte[5] = (uint16_t)_roboclaw.getc();
//   read_byte[6] = (uint16_t)_roboclaw.getc();

//   int32_t enc1;

//   enc1 = read_byte[1] << 24;
//   enc1 |= read_byte[2] << 16;
//   enc1 |= read_byte[3] << 8;
//   enc1 |= read_byte[4];

//   // int32_t enc1 = (read_byte[3] << 24) | (read_byte[2] << 16) | (read_byte[1] << 8) | read_byte[0];

//   // printf("ENC  %lu\n", enc1);

//   return enc1;
// }

uint32_t RoboClaw::ReadEncM1(uint8_t *status, bool *valid)
{
  return Read4_1(address, GETM1ENC, status, valid);
}

void RoboClaw::flush()
{
  while (_roboclaw.readable())
  {
    _roboclaw.getc();
  }

  return;
}

uint32_t RoboClaw::Read4_1(uint8_t address, uint8_t cmd, uint8_t *status, bool *valid)
{
  // uint8_t crc;

  if (valid)
    *valid = false;

  uint32_t value = 0;
  // uint8_t trys = MAXRETRY;
  uint8_t trys = 2;
  int16_t data;
  do
  {
    flush();

    crc_clear();
    _roboclaw.putc(address);
    crc_update(address);
    _roboclaw.putc(cmd);
    crc_update(cmd);

    data = read();
    crc_update(data);
    value = (uint32_t)data << 24;

    if (data != -1)
    {
      data = read();
      crc_update(data);
      value |= (uint32_t)data << 16;
    }

    if (data != -1)
    {
      data = read();
      crc_update(data);
      value |= (uint32_t)data << 8;
    }

    if (data != -1)
    {
      data = read();
      crc_update(data);
      value |= (uint32_t)data;
    }

    if (data != -1)
    {
      data = read();
      crc_update(data);
      if (status)
        *status = data;
    }

    // printf("value before crc %lu\n", value);

    if (data != -1)
    {
      Thread::wait(1); // does not work without for whatever reason
      uint16_t ccrc;
      data = read();
      if (data != -1)
      {
        ccrc = data << 8;
        data = read();
        if (data != -1)
        {
          ccrc |= data;
          if (crc_get() == ccrc)
          {
            *valid = true;
            return value;
          }
        }
      }
    }
  } while (trys--);

  return false;
}

uint16_t RoboClaw::read(int timeout)
{
  // Timer timer;
  readTimer.reset();
  readTimer.start();

  int cycles = 0;

  while (!_roboclaw.readable())
  {
    cycles++;

    if (readTimer.read_ms() >= timeout)
    {
      // printf("GIVE UP %d\n", cycles);
      return -1;
    }
  }

  if (cycles > 0)
  {
    // printf("WAITED %d\n", cycles);
  }

  return (uint16_t)_roboclaw.getc();
}

// uint32_t RoboClaw::read4_1(uint8_t address, uint8_t cmd, uint8_t *status, bool *valid)
// {
//   if (valid)
//     *valid = false;

//   uint16_t value = 0;
//   uint8_t trys = 2;
//   int16_t data;
//   do
//   {
//     // flush();
//     // _roboclaw.flush();

//     crc_clear();
//     // _roboclaw.putc(address);
//     _roboclaw.putc(address);
//     crc_update(address);
//     // write(cmd);
//     _roboclaw.putc(cmd);
//     crc_update(cmd);

//     data = (uint16_t)_roboclaw.getc();
//     crc_update(data);
//     value = (uint16_t)data << 24;

//     if (data != -1)
//     {
//       data = (uint16_t)_roboclaw.getc();
//       crc_update(data);
//       value = (uint16_t)data << 16;
//     }

//     if (data != -1)
//     {
//       data = (uint16_t)_roboclaw.getc();
//       crc_update(data);
//       value = (uint16_t)data << 8;
//     }

//     if (data != -1)
//     {
//       data = (uint16_t)_roboclaw.getc();
//       crc_update(data);
//       value |= (uint16_t)data;
//     }

//     if (data != -1)
//     {
//       data = (uint16_t)_roboclaw.getc();
//       crc_update(data);
//       if (status)
//         *status = data;
//     }

//     if (data != -1)
//     {
//       uint16_t ccrc;
//       data = (uint16_t)_roboclaw.getc();
//       if (data != -1)
//       {
//         ccrc = data << 8;
//         data = (uint16_t)_roboclaw.getc();
//         if (data != -1)
//         {
//           ccrc |= data;
//           if (crc_get() == ccrc)
//           {
//             *valid = true;
//             return value;
//           }
//         }
//       }
//     }
//   } while (trys--);

//   return false;
// }

int32_t RoboClaw::ReadEncM2()
{
  int32_t enc2;
  uint16_t read_byte2[7];
  write_(GETM2ENC, 0x00, true, false);

  read_byte2[0] = (uint16_t)_roboclaw.getc();
  read_byte2[1] = (uint16_t)_roboclaw.getc();
  read_byte2[2] = (uint16_t)_roboclaw.getc();
  read_byte2[3] = (uint16_t)_roboclaw.getc();
  read_byte2[4] = (uint16_t)_roboclaw.getc();
  read_byte2[5] = (uint16_t)_roboclaw.getc();
  read_byte2[6] = (uint16_t)_roboclaw.getc();

  enc2 = read_byte2[1] << 24;
  enc2 |= read_byte2[2] << 16;
  enc2 |= read_byte2[3] << 8;
  enc2 |= read_byte2[4];

  return enc2;
}

int32_t RoboClaw::ReadSpeedM1()
{
  int32_t speed1;
  uint16_t read_byte[7];
  write_n(2, address, GETM1SPEED);

  read_byte[0] = (uint16_t)_roboclaw.getc();
  read_byte[1] = (uint16_t)_roboclaw.getc();
  read_byte[2] = (uint16_t)_roboclaw.getc();
  read_byte[3] = (uint16_t)_roboclaw.getc();
  read_byte[4] = (uint16_t)_roboclaw.getc();
  read_byte[5] = (uint16_t)_roboclaw.getc();
  read_byte[6] = (uint16_t)_roboclaw.getc();

  speed1 = read_byte[1] << 24;
  speed1 |= read_byte[2] << 16;
  speed1 |= read_byte[3] << 8;
  speed1 |= read_byte[4];

  return speed1;
}

int32_t RoboClaw::ReadSpeedM2()
{
  int32_t speed2;
  uint16_t read_byte2[7];
  write_n(2, address, GETM2SPEED);

  read_byte2[0] = (uint16_t)_roboclaw.getc();
  read_byte2[1] = (uint16_t)_roboclaw.getc();
  read_byte2[2] = (uint16_t)_roboclaw.getc();
  read_byte2[3] = (uint16_t)_roboclaw.getc();
  read_byte2[4] = (uint16_t)_roboclaw.getc();
  read_byte2[5] = (uint16_t)_roboclaw.getc();
  read_byte2[6] = (uint16_t)_roboclaw.getc();

  speed2 = read_byte2[1] << 24;
  speed2 |= read_byte2[2] << 16;
  speed2 |= read_byte2[3] << 8;
  speed2 |= read_byte2[4];

  return speed2;
}

void RoboClaw::ResetEnc()
{
  write_n(2, address, RESETENC);
}

void RoboClaw::SpeedM1(int32_t speed)
{
  write_n(6, address, M1SPEED, SetDWORDval(speed));
}

void RoboClaw::SpeedM2(int32_t speed)
{
  write_n(6, address, M2SPEED, SetDWORDval(speed));
}

void RoboClaw::SpeedAccelM1(int32_t accel, int32_t speed)
{
  write_n(10, address, M1SPEEDACCEL, SetDWORDval(accel), SetDWORDval(speed));
}

void RoboClaw::SpeedAccelM2(int32_t accel, int32_t speed)
{
  write_n(10, address, M2SPEEDACCEL, SetDWORDval(accel), SetDWORDval(speed));
}

void RoboClaw::SpeedAccelM1M2(int32_t accel, int32_t speed1, int32_t speed2)
{
  write_n(14, address, MIXEDSPEEDACCEL, SetDWORDval(accel), SetDWORDval(speed1), SetDWORDval(speed2));
}

void RoboClaw::SpeedDistanceM1(int32_t speed, uint32_t distance, uint8_t buffer)
{
  write_n(11, address, M1SPEEDDIST, SetDWORDval(speed), SetDWORDval(distance), buffer);
}

void RoboClaw::SpeedDistanceM2(int32_t speed, uint32_t distance, uint8_t buffer)
{
  write_n(11, address, M2SPEEDDIST, SetDWORDval(speed), SetDWORDval(distance), buffer);
}

void RoboClaw::SpeedAccelDistanceM1(int32_t accel, int32_t speed, uint32_t distance, uint8_t buffer)
{
  write_n(15, address, M1SPEEDACCELDIST, SetDWORDval(accel), SetDWORDval(speed), SetDWORDval(distance), buffer);
}

void RoboClaw::SpeedAccelDistanceM2(int32_t accel, int32_t speed, uint32_t distance, uint8_t buffer)
{
  write_n(15, address, M2SPEEDACCELDIST, SetDWORDval(accel), SetDWORDval(speed), SetDWORDval(distance), buffer);
}

void RoboClaw::SpeedAccelDeccelPositionM1(uint32_t accel, int32_t speed, uint32_t deccel, int32_t position, uint8_t flag)
{
  write_n(19, address, M1SPEEDACCELDECCELPOS, SetDWORDval(accel), SetDWORDval(speed), SetDWORDval(deccel), SetDWORDval(position), flag);
}

void RoboClaw::SpeedAccelDeccelPositionM2(uint32_t accel, int32_t speed, uint32_t deccel, int32_t position, uint8_t flag)
{
  write_n(19, address, M2SPEEDACCELDECCELPOS, SetDWORDval(accel), SetDWORDval(speed), SetDWORDval(deccel), SetDWORDval(position), flag);
}

void RoboClaw::SpeedAccelDeccelPositionM1M2(uint32_t accel1, uint32_t speed1, uint32_t deccel1, int32_t position1, uint32_t accel2, uint32_t speed2, uint32_t deccel2, int32_t position2, uint8_t flag)
{
  write_n(35, address, MIXEDSPEEDACCELDECCELPOS, SetDWORDval(accel1), SetDWORDval(speed1), SetDWORDval(deccel1), SetDWORDval(position1), SetDWORDval(accel2), SetDWORDval(speed2), SetDWORDval(deccel2), SetDWORDval(position2), flag);
}
