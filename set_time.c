#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include "set_time.h"

#define DEVICE_NAME "TIME"// Имя создаваемого устройства, расположенного в /dev/
#define BUF_LEN 80        // Длина буфера

static int Device_Open = 0;

static dev_t first; // Первый номер устройства
static struct cdev c_dev; // Структура символьного устройства
static struct class *cl; // Класс драйвера

static char msg[BUF_LEN];
static char *msg_Ptr;

// Определение функций, поддерживаемых устройствоом
// Инициализация устройства
static int device_open(struct inode *, struct file *);
// Деинициализация устройства
static int device_release(struct inode *, struct file *);
// Получение данных от устройства
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
// Отправление данных на устройство
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static long dev_ioctl(struct file *, unsigned int,unsigned long);

// Файловые операции поддерживаемые устройством
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = dev_ioctl,
    .open = device_open,
    .release = device_release
};

// Функция инициализации модуля
static int __init time(void)
{
  // Регистрация символьного устройства
  // И создание файла устройства

    // Выделение памяти
    if (alloc_chrdev_region(&first, 0, 1, DEVICE_NAME) < 0) return -1;
    // Регистрация класса
    if ((cl = class_create(THIS_MODULE, DEVICE_NAME)) == NULL)
  {
    unregister_chrdev_region(first, 1);
    return -1;
  }
    // Создание девайса
    if (device_create(cl, NULL, first, NULL, DEVICE_NAME) == NULL)
  {
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }
    // Инициализация устройства
    cdev_init(&c_dev, &fops);
    // Добавление устройства
    if (cdev_add(&c_dev, first, 1) == -1)
  {
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }

  printk(KERN_INFO "/dev/%s c %d 0", DEVICE_NAME, MAJOR(first));

  return 0;
}

// Функция выгрузки модуля
static void __exit cleanup_time(void)
{
  device_destroy(cl, first);
  class_destroy(cl);
  unregister_chrdev_region(first, 1);
  printk( KERN_INFO "UNLOADING MODULE" );
}

static int device_open(struct inode *inode, struct file *file)
{

  if (Device_Open)
      return -EBUSY;

  Device_Open++;

  msg_Ptr = msg;
  try_module_get(THIS_MODULE);

  return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--;

    module_put(THIS_MODULE);

    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length,
                                                                loff_t * offset)
{
    int bytes_read = 0;


    if (*msg_Ptr == 0)
        return 0;

    while (length && *msg_Ptr) {

        put_user(*(msg_Ptr++), buffer++);

        length--;
        bytes_read++;
    }

    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len,
                                                                   loff_t * off)
{
    int nbytes = len - copy_from_user(msg_Ptr + *off, buff, len);
    *off += nbytes;

    return nbytes;
}

static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
  int ret = 0;
   switch (cmd) {
     case TIME_IOCSET:
          ret = set_date_time((struct my_date *)arg);
          break;
     case TIME_IOCREAD:
          ret = read_time((struct my_date *)arg);
    /*
        Для других функций
    */
     default:
          return -EINVAL;
   }

   return ret;
}

int set_date_time(struct my_date *users_date)
{
  struct timeval time;
  struct timespec tp;

  do_gettimeofday(&time);

  tp.tv_sec = mktime64(users_date->year,users_date->mon,users_date->day,
                              users_date->hour,users_date->min,users_date->sec);
  tp.tv_nsec = time.tv_usec;

  do_settimeofday(&tp);

  return 0;
}

int read_time(struct my_date *users_date)
{

    struct tm t2;
    struct timeval time;

    do_gettimeofday(&time);

    time_to_tm(time.tv_sec, 0, &t2);

    users_date->year = t2.tm_year + 1900;
    users_date->mon = t2.tm_mon + 1;
    users_date->day = t2.tm_mday;
    users_date->hour = t2.tm_hour;
    users_date->min = t2.tm_min;
    users_date->sec = t2.tm_sec;

    return 0;
}

module_init(time);
module_exit(cleanup_time);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chesnokov Evgeny");
MODULE_DESCRIPTION("Linux time module.");
MODULE_VERSION("4.0");
