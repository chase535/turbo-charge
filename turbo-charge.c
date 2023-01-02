#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void strrpc(char *str, char *oldstr, char *newstr)
{
	char bstr[strlen(str)];
	memset(bstr, 0, sizeof(bstr));
	for(int i = 0; i < strlen(str); i++)
	{
		if(!strncmp(str+i, oldstr, strlen(oldstr)))
		{
			strcat(bstr, newstr);
			i += strlen(oldstr) - 1;
		}
		else
		{
			strncat(bstr, str + i, 1);
		}
	}
	strcpy(str, bstr);
}

void fclose_file(FILE *ffile)
{
	if(ffile != NULL)
	{
		fclose(ffile);
	}
}

void pclose_file(FILE *pfile)
{
	if(pfile != NULL)
	{
		pclose(pfile);
	}
}

void set_value(char *abc, char *numb)
{
	FILE *fn;
	if(abc != NULL)
	{
		if(access(abc, W_OK) == 0)
		{
			fn = fopen(abc, "wt");
			if(fn != NULL)
			{
				fputs(numb, fn);
				fclose_file(fn);
				fn = NULL;
			}
		}
	}
}

void line_feed(char *line)
{
	char *p;
	if((p = strchr(line, '\n')) != NULL)
	{
		*p = '\0';
	}
}

void charge_value(char *i)
{
	set_value("/sys/class/power_supply/battery/charging_enabled", i);
	set_value("/sys/class/power_supply/battery/battery_charging_enabled", i);
	if(srtcmp(i,"1") == 0)
	{
		set_value("/sys/class/power_supply/battery/input_suspend", "0");
		set_value("/sys/class/qcom-battery/restricted_charging", "0");
	}
	else if(srtcmp(i,"0") == 0)
	{
		set_value("/sys/class/power_supply/battery/input_suspend", "1");
		set_value("/sys/class/qcom-battery/restricted_charging", "1");
	}
}

int main()
{
	FILE *fp,*fq,*fm,*fa,*fb,*fc,*fd,*fe,*ff;
	char done[100],asdf[310],charge[100],uevent[3010],power[100],current_max[100],highest_temp_current[100],buffer[3010],constants[3010],msg[110],thermal[310],temps[3010],*p,*q,*m,*n,option[1010];
	int done_int,asdf_int,wasd=0,charge_start,charge_stop,temp_ctrl,power_ctrl,power_int,recharge_temp,current_max_int,temp_max,highest_temp_current_int,temp_int,qwer;
	fp = popen("ls /sys/class/thermal/thermal_*/type", "r");
	while (fgets(buffer, 3000, fp) != NULL)
	{
		line_feed(buffer);
		fq = fopen(buffer, "rt");
		fgets(msg, 100, fq);
		fclose_file(fq);
		fq = NULL;
		line_feed(msg);
		if(strcmp(msg, "conn_therm") == 0)
		{
			strrpc(buffer, "type", "temp");
			break;
		}
	}
	if(buffer == NULL)
	{
		printf("获取温度失败！请联系模块制作者！");
		exit(2);
	}
	pclose_file(fp);
	fp = NULL;
	charge_value("1");
	while(1)
	{
		if(access("/data/adb/turbo-charge/option.txt", F_OK) == -1)
		{
			printf("配置文件丢失！请联系模块制作者！");
			exit(1);
		}
		set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
		set_value("/sys/kernel/fast_charge/force_fast_charge", "1");
		set_value("/sys/class/power_supply/battery/system_temp_level", "1");
		set_value("/sys/kernel/fast_charge/failsafe", "1");
		set_value("/sys/class/power_supply/battery/allow_hvdcp3", "1");
		set_value("/sys/class/power_supply/usb/pd_allowed", "1");
		set_value("/sys/class/power_supply/battery/subsystem/usb/pd_allowed", "1");
		set_value("/sys/class/power_supply/battery/input_current_limited", "0");
		set_value("/sys/class/power_supply/battery/input_current_settled", "1");
		set_value("/sys/class/qcom-battery/restrict_chg", "0");
		fe = fopen("/sys/class/power_supply/battery/uevent", "rt");
		while (fgets(uevent, 3000, fe) != NULL)
		{
			sscanf(uevent, "POWER_SUPPLY_STATUS=%s", charge);
		}
		if(strcmp(charge, "Charging") == 0)
		{
			fb = popen("ls /sys/class/power_supply/*/*temp", "r");
			while (fgets(temps, 3000, fb) != NULL)
			{
				line_feed(temps);
				set_value(temps, "280");
			}
			pclose_file(fb);
			fb = NULL;
			wasd = 1;
		}
		else
		{
			if(wasd == 1)
			{
				fb = popen("ls /sys/class/power_supply/*/*temp", "r");
				while (fgets(temps, 3000, fb) != NULL)
				{
					line_feed(temps);
					fm = fopen(buffer, "rt");
					fscanf(fm, "%c%c%c", &asdf[0],&asdf[1],&asdf[2]);
					asdf_int = atoi(asdf);
					asdf_int >= 550?set_value(temps, "280"):set_value(temps, asdf);
					fclose_file(fm);
					fm = NULL;
				}
				pclose_file(fb);
				fb = NULL;
			}
		}
		fclose_file(fe);
		fe = NULL;
		fc = fopen("/data/adb/turbo-charge/option.txt", "rt");
		while(fgets(option, 1000, fc) != NULL)
		{
			sscanf(option, "TEMP_CTRL=%d", &temp_ctrl);
			sscanf(option, "POWER_CTRL=%d", &power_ctrl);
			sscanf(option, "CHARGE_START=%d", &charge_start);
			sscanf(option, "CHARGE_STOP=%d", &charge_stop);
			sscanf(option, "CURRENT_MAX=%d", &current_max_int);
			sscanf(option, "TEMP_MAX=%d", &temp_max);
			sscanf(option, "HIGHEST_TEMP_CURRENT=%d", &highest_temp_current_int);
			sscanf(option, "RECHARGE_TEMP=%d", &recharge_temp);
		}
		if(power_ctrl == 1)
		{
			fd = fopen("/sys/class/power_supply/battery/capacity", "rt");
			fgets(power, 90, fd);
			power_int = atoi(power);
			if(power_int >= charge_stop)
			{
				if(charge_stop == 100)
				{
					fm = fopen("/sys/class/power_supply/battery/current_now", "rt");
					fgets(done, 300, fm);
					done_int = atoi(done);
					if(done_int == 0)
					{
						charge_value("0");
						qwer = 1;
					}
					fclose_file(fm);
					fm = NULL;
				}
				else
				{
					charge_value("0");
					qwer = 1;
				}
			}
			if(power_int <= charge_start)
			{
				charge_value("1");
				qwer = 0;
			}
		}
		else
		{
			if(qwer == 1)
			{
				charge_value("1");
				qwer = 0;
			}
			
		}
		fclose_file(fd);
		fd = NULL;
		if(temp_ctrl == 1)
		{
			fm = fopen(buffer, "rt");
			fgets(thermal, 300, fm);
			temp_int = atoi(thermal);
			sprintf(highest_temp_current, "%d", highest_temp_current_int);
			sprintf(current_max, "%d", current_max_int);
			sleep(1);
			if(temp_int > temp_max*1000)
			{
				while(temp_int > recharge_temp*1000)
				{
					fclose_file(fm);
					fm = NULL;
					fm = fopen(buffer, "rt");
					fgets(thermal, 300, fm);
					temp_int = atoi(thermal);
					sleep(1);
					fclose_file(fc);
					fc = NULL;
					fc = fopen("/data/adb/turbo-charge/option.txt", "rt");
					while(fgets(option, 1000, fc) != NULL)
					{
						sscanf(option, "TEMP_CTRL=%d", &temp_ctrl);
						sscanf(option, "CURRENT_MAX=%d", &current_max_int);
						sscanf(option, "TEMP_MAX=%d", &temp_max);
						sscanf(option, "HIGHEST_TEMP_CURRENT=%d", &highest_temp_current_int);
						sscanf(option, "RECHARGE_TEMP=%d", &recharge_temp);
					}
					fclose_file(fc);
					fc = NULL;
					sprintf(highest_temp_current, "%d", highest_temp_current_int);
					if(temp_ctrl == 0) break;
					fa = popen("ls /sys/class/power_supply/*/constant_charge_current_max", "r");
					while (fgets(constants, 3000, fa) != NULL)
					{
						line_feed(constants);
						set_value(constants, highest_temp_current);
					}
					pclose_file(fa);
					fa = NULL;
				}
			}
			fa = popen("ls /sys/class/power_supply/*/constant_charge_current_max", "r");
			while (fgets(constants, 3000, fa) != NULL)
			{
				line_feed(constants);
				set_value(constants, current_max);
			}
			fclose_file(fm);
			fm = NULL;
			pclose_file(fa);
			fa = NULL;
			fclose_file(fc);
			fc = NULL;
		}
		else
		{
			sleep(1);
			sprintf(current_max, "%d", current_max_int);
			fa = popen("ls /sys/class/power_supply/*/constant_charge_current_max", "r");
			while (fgets(constants, 3000, fa) != NULL)
			{	
				line_feed(constants);
				set_value(constants, current_max);
			}
			pclose_file(fa);
			fa = NULL;
			fclose_file(fc);
			fc = NULL;
		}
	}
	return 0;
}
