/* Filename: led_module.c
   Description: loadable module for led driver
   Author : Vihanga Bare
*/


#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h> 
#include<linux/gpio.h>
#include<linux/timer.h>
#include<uapi/asm-generic/errno-base.h>
#include<linux/kernel.h>
#include<asm/uaccess.h>
#include<linux/slab.h>
#include<linux/linkage.h>
#include<linux/syscalls.h>

#define DEVICE_NAME 	"SimpleCharDrv"
#define LED_PIN 	53
#define PIN_DIR_OUT	1
#define BUFLEN		256

MODULE_LICENSE("GPL");

static struct timer_list my_timer;
static struct cdev *simplechardrv;
dev_t mychardevice;
static int32_t major_num; //device major number
static int32_t minor_num; //device minor number
static int32_t ret;
static int32_t ledON; //led state
static int32_t recv_len;
static long ontime;
static int32_t timerstate;

typedef struct data
{
	bool state;
	long period;
	long duty;
}data_t;
data_t buffer; //buffer to send data to and from user space 

/* function prototypes */

ssize_t char_module_read(struct file *read_file, char __user *read_user, size_t len, loff_t *read_offset);
ssize_t char_module_write(struct file *write_file, const char __user *write_user, size_t len, loff_t *write_offset);
ssize_t char_module_write(struct file *write_file, const char __user *write_user, size_t len, loff_t *write_offset);
int char_module_open(struct inode *open_inode, struct file *open_file);
int char_module_close(struct inode *close_inode, struct file *close_file);

/* file operations */
struct file_operations char_module_file_operations = {
	.owner 	 = THIS_MODULE,
	.open 	 = char_module_open,
	.read	 = char_module_read,
	.write	 = char_module_write,
	.release = char_module_close,
};

/*
Timer callback function
*/

void my_timer_callback(unsigned long data)
{
        static int timer_count = 0;
		
	printk("Restarting timer........%d \n",jiffies_to_msecs(jiffies));

        ret = mod_timer(&my_timer,jiffies + msecs_to_jiffies(buffer.period - ontime));
        if(ret)
        {
                printk("mod_timer returns 1");
        }
	timer_count++;
	if(timer_count % 2 != 0)
	{
		ledON = !ledON;
	}
	gpio_set_value(LED_PIN,ledON);
	printk(" Timer count....%d \n",timer_count);
}
/* read file operation for char driver*/
ssize_t char_module_read(struct file *read_file, char __user *read_user, size_t len, loff_t *read_offset)
{
	int32_t msglen = recv_len;
	int32_t count = 0;
	//default values for first read
	buffer.state = false;
	buffer.period = 0;
	buffer.duty = 0;
	
	printk(KERN_ALERT"Inside the char_module_read funtion\n");	
	count = copy_to_user(read_user,&buffer,msglen);
   	if (count == 0)
	{            
      		printk(KERN_INFO " Sent %d characters to the user\n",msglen);
		printk(KERN_ALERT"data to user space state = %d\n",buffer.state);
        	printk(KERN_ALERT"data to user space period = %ld\n",buffer.period);
        	printk(KERN_ALERT"data to user space duty-cycle = %ld\n",buffer.duty);
		return msglen;
   	}
   	else if (count < 0) 
	{
      		printk(KERN_INFO "Failed to send %d characters to the user\n",count);
      		return -EFAULT;
	}             
 
	return 0;
}
/*write file operation for char driver */
ssize_t char_module_write(struct file *write_file, const char __user *write_user, size_t len, loff_t *write_offset)
{
	int32_t count;
	int32_t buflen;
	buflen = len;
	recv_len = len;
	
	timerstate = 1; //default is on
	printk(KERN_ALERT"Inside the char_module_write funtion\n");
	if (!gpio_is_valid(LED_PIN))
	{
      		printk(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
   	}
	
	if(buflen <= 0)
	{
		printk(KERN_DEBUG "Invalid length of buffer.......\n"); 
		/* return error code for invalid length */
		return EINVAL;
	}
	
	count = copy_from_user(&buffer,write_user,buflen); 
	/*copy from user copies from user space buffer in_buf to kernel space buffer kbuffer*/
	if(count > 0)
	{
		printk(KERN_DEBUG "No. of bytes not copied from user space......%d", count);
		/* copy from user returns the no. of bytes which were not copied from user space to kernel space  */
		return ENOMEM; 
		/* return error code if copy form user fails */
	}
	printk(KERN_ALERT"data received from user space state = %d\n",buffer.state);
	printk(KERN_ALERT"data received from user space period = %ld\n",buffer.period);
	printk(KERN_ALERT"data received from user space duty-cycle = %ld\n",buffer.duty);
	
	/* request the control over GPIO LED  */
	gpio_request(LED_PIN,"Vihanga");
	
	/* set direction to output */
	ledON = buffer.state;
	gpio_direction_output(LED_PIN,true);
	
	/* set value of gpio */ 
	gpio_set_value(LED_PIN,ledON);	
	if(buffer.state == false)
		timerstate = 0;
	
	printk(KERN_ALERT "Installing Timer Module .........\n");
	setup_timer(&my_timer,my_timer_callback,0);
        if(timerstate == 1)
	{
		printk("Starting the timer to fire %d \n",jiffies_to_msecs(jiffies));
	
		ontime = (buffer.duty * buffer.period)/100;
        	ret = mod_timer(&my_timer,jiffies + msecs_to_jiffies(ontime));
        	if(ret)
        	{
                	printk(KERN_ALERT "mod_timer returns 1");
			return 1;
        	}
	}
	else
	{
		 gpio_set_value(LED_PIN,false);
	}
	return len;
}
/* open file operations */
int char_module_open(struct inode *open_inode, struct file *open_file)
{
	printk(KERN_ALERT"Inside the char_module_open funtion\n");
	return 0;

}
int char_module_close(struct inode *close_inode, struct file *close_file)
{
	printk(KERN_ALERT"Inside the char_module_close funtion\n");
	return 0;
}

/* char driver init function */
int __init char_module_init(void)
{
	printk(KERN_ALERT"Inside the char_module_init funtion\n");

	/* allocate memory for char driver */
	ret = alloc_chrdev_region(&mychardevice,0,1,DEVICE_NAME);
	if(ret < 0)
	{
		printk(KERN_ALERT" failed to allocate a major number\n");
		return ret;
	}
	major_num = MAJOR(mychardevice);

	printk(KERN_ALERT"Major Number : %d\n",major_num);
	
	/* once driver has been allocated */
	simplechardrv = cdev_alloc();
	simplechardrv->ops = &char_module_file_operations;
	simplechardrv->owner = THIS_MODULE;
	
	ret = cdev_add(simplechardrv,mychardevice,1);
	if(ret < 0)
	{
		printk(KERN_ALERT"unable to add char device to kernel\n");	
		return ret;
	}

	return 0;
}
/* char module exit function */
void char_module_exit(void)
{
	printk(KERN_ALERT"Inside char_module_exit function\n");
	printk(KERN_ALERT"Unexport LED GPIO \n");
	
	printk(KERN_ALERT"Free LED GPIO \n");
	gpio_free(LED_PIN);
	unregister_chrdev(major_num,DEVICE_NAME);
        
        ret = del_timer(&my_timer);
        if(ret)
        {	
		printk(KERN_ALERT "del_timer returns 1 \n");
		return;
	}
	printk(KERN_ALERT "Removing Timer\n");
}

module_init(char_module_init);
module_exit(char_module_exit);
