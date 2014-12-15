/**
 * IR decoder for NEC.
 * Copyright (C) 2014 Changli Gao <xiaosuo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define IR_IN  8

#define IN_RANGE(x, low, high) ((x) >= (low) && (x) <= (high))
#define IS_560US(width) IN_RANGE(width, 6, 13)
#define IS_1560US(width) IN_RANGE(width, 23, 28)
#define IS_9MS(width) IN_RANGE(width, 137, 144)
#define IS_4500US(width) IN_RANGE(width, 68, 72)
#define IS_2250US(width) IN_RANGE(width, 33, 36)

#if 0
#define DEBUG(depth, width) debug((depth), (width))
#else
#define DEBUG(depth, width) do {} while (0)
#endif

int ir_code = 0x00;
int ir_addr = 0x00;

void init_timer1(void)
{
  TCCR1A = 0X00;
  TCCR1B = 0X05;
  TCCR1C = 0X00;
  TCNT1 = 0X00;
  TIMSK1 = 0X00;
}

int get_level_width(int level)
{
  int width;

  while (digitalRead(IR_IN) == level)
    /* empty */;
  width = TCNT1;
  TCNT1 = 0;

  return width;
}

void debug(int depth, int width)
{
  int i;
  for (i = 0; i < depth; ++i)
    Serial.print('-');
  Serial.println(width);
}

char get_bit()
{
  int width = get_level_width(LOW);
  if (IS_560US(width)) {
    width = get_level_width(HIGH);
    if (IS_560US(width))
      return 0;
    else if (IS_1560US(width))
      return 1;
  }
  DEBUG(3, width);
  return -1;
}

int get_byte()
{
  int i, retval = 0;

  for (i = 0; i < 8; ++i) {
    switch (get_bit()) {
    case 1:
      retval |= (1 << i);
      break;
    case 0:
      break;
    default:
      return -1;  
    }
  }

  return retval;
}

int __get_addr_and_comm()
{
  int comp;

  ir_addr = get_byte();
  if (ir_addr < 0)
    return -1;
  comp = get_byte();
  if (comp < 0)
    return -1;
#if 1
  ir_addr |= comp << 8;
#else
  if (ir_addr != ((~comp) & 0xff))
    return -1; 
#endif

  ir_code = get_byte();
  if (ir_code < 0)
    return -1;
  comp = get_byte();
  if (comp < 0)
    return -1;
  if (ir_code != ((~comp) & 0xff))
    return -1;
  return 0;
}

int get_addr_and_comm(void)
{
  int width;

  TCNT1 = 0;
  width = get_level_width(LOW);
  if (IS_9MS(width)) {
    width = get_level_width(HIGH);
    if (IS_4500US(width)) {
      return __get_addr_and_comm();
    } else if (IS_2250US(width)) {
      width = get_level_width(LOW);
      if (!IS_560US(width)) {
        DEBUG(3, width);
      }
    } else {
      DEBUG(2, width);
    }
  } else {
    DEBUG(1, width);
  }

  return -1;
}

void setup()
{
  pinMode(IR_IN, INPUT);
  Serial.begin(9600);
  init_timer1();
}

void loop()
{ 
  if (digitalRead(IR_IN) == LOW) {
    if (get_addr_and_comm() == 0) {
      Serial.print('+');
      Serial.print(ir_addr & 0xffff, HEX);
      Serial.print(':');
      Serial.println(ir_code, HEX);
    }
  }
}
