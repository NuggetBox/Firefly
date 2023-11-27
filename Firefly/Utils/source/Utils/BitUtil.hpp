#pragma once

#define READ_BIT(data, index) (((data) >> (index)) & 1)
#define SET_BIT(data, index) ((data) |= (1 << (index)))
#define CLEAR_BIT(data, index) ((data) &= ~(1 << (index)))
#define FLIP_BIT(data, index) ((data) ^= READ_BIT(index))
#define FLIP_BITS(data) ((data) =~ (data))