#ifndef SET_TIME_H
#define SET_TIME_H

typedef struct my_date {
    unsigned int sec, min, hour; // поля структуры определяющие время
    unsigned int year, mon, day; // поля структуры определяющие дату
} my_date;

// Определение "Магических" чисел для ioctl
#define TIME_IOC_MAGIC 'k'
#define TIME_IOCSET _IOWR(TIME_IOC_MAGIC, 1, struct my_date *)
#define TIME_IOCREAD _IOWR(TIME_IOC_MAGIC, 2, struct my_date *)

/*
  Следующие "магические" числа для разных функций
*/

int set_date_time(struct my_date *);

int read_time(struct my_date *);

#endif
