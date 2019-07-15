#ifndef _FW_DEVICE_H_
#define _FW_DEVICE_H_

class CDevice
{
public:
	CDevice():_bOpen(false){}
	virtual ~CDevice(){}

	//enable state
	void setOpen(bool open){_bOpen = open;}
	bool isOpen()
	{
		return _bOpen;
	}

	//if succeed, return 0
	virtual int doInit(){return 0;}
	virtual int doSelfChecking(){return 0;}
	virtual int doRun(){return 0;}

private:
	bool _bOpen;	//if the device can be initialized successfully, _bOpen is TRUE
};


#endif	//_FW_DEVICE_H_
