#include <rtthread.h>

#include <speex/speex.h>

void speex_warning_int(const char *str, int val)
{
   rt_kprintf ("warning: %s %d\n", str, val);
}

void speex_warning(const char *str)
{
   rt_kprintf ("warning: %s \n", str);
}

void speex_notify(const char *str)
{
   rt_kprintf ("speex_notify: %s \n", str);
}

/**
  * @brief  Ovveride the _speex_fatal function of the speex library
  * @param  None
  * @retval : None
  */
void _speex_fatal(const char *str, const char *file, int line)
{
  rt_kprintf ("_speex_fatal: %s \n", str);
}

/**
  * @brief  Ovveride the _speex_putc function of the speex library
  * @param  None
  * @retval : None
  */
void _speex_putc(int ch, void *file)
{
  rt_kprintf ("_speex_putc: %c \n", ch);
}
