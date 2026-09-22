#ifndef __PID_H
#define __PID_H

typedef struct {
	float Target;			//设定值
	float Actual;			//实际值
	float Actual1;		//积分先行的上次实际值
	float Out;				//输出值
	
	float Kp;
	float Ki;
	float Kd;
	
	float Error0;			//本次误差
	float Error1;			//上次误差
	float ErrorInt;		//累计误差
	
	float ErrorIntMax;//积分限幅
	float ErrorIntMin;//积分限幅
	
	float OutMax;
	float OutMin;
	
	float Outoffset;
} PID_t;

void PID_Update(volatile PID_t *p);
void PID_Init (volatile PID_t *p);

#endif
