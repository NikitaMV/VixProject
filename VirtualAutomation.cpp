#include <iostream>
#include <fstream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <cstring>
#include "vix.h"
#include <curl/curl.h>

using namespace std;

#define  CONNTYPE    VIX_SERVICEPROVIDER_VMWARE_WORKSTATION
#define  HOSTNAME ""
#define  HOSTPORT 0
#define  USERNAME ""
#define  PASSWORD ""
#define  VMPOWEROPTIONS  0
#define  CRT_SECURE_NO_WARNINGS


struct FtpFile {
	const char *filename;
	FILE *stream;
};

static size_t save_file(void *buffer, size_t size, size_t nmemb, void *stream)
{
	struct FtpFile *out = (struct FtpFile *)stream;
	if (out && !out->stream) 
	{		
		out->stream = fopen(out->filename, "wb");
		if (!out->stream)
			return -1; 
	}
	return fwrite(buffer, size, nmemb, out->stream);
}


int main(int argc, char** argv)
{
	setlocale(LC_ALL, "russian");

	if (argc == 1)
	{
		cout << "Отсутствуют параметры запуска. Укажите параметр -h для вызова справки" << endl;
		return 0;		
	}	

	if (argc == 2 && !strcmp(argv[1], "-h"))
	{
		cout << "\nСПРАВКА" << endl << endl;
		cout << "Формат запуска приложения: vm -d destination_of_VM -u user -p password -b browser -v version -c capacity [-l language] " << endl << endl;
		cout << "destination_of_VM \t- путь в файлу .vmx виртуальной машины." << endl;
		cout << "user \t\t\t- логин для виртуальной машины" << endl;
		cout << "password \t\t- пароль для виртуальной машины" << endl;
		cout << "browser \t\t- устанавливаемый браузер. Есть варианты: opera, firefox, chrome" << endl;
		cout << "version \t\t- версия браузера. Для браузера chrome доступна только версия last(последняя)" << endl;
		cout << "capacity \t\t- разрядность браузера. Может быть 32 или 64" << endl;
		cout << "language \t\t- язык браузера. Параметр доступен только для firefox.  По умолчанию en-US" << endl << endl;
		cout << "Пример запуска: vm -d \"D:\\\\VMs\\\\Windows_7\\\\Windows_7.vmx\" -u nikita -p 12345 -b opera -v 38.0.2220.31 -c 32" << endl;
		
		return 0;
	}	

	if (argc == 13 && (strcmp(argv[1], "-d") || strcmp(argv[3], "-u") || strcmp(argv[5], "-p") || strcmp(argv[7], "-b") || strcmp(argv[9], "-v") || strcmp(argv[11], "-c")))
	{
		cout << "Указаны неверные параметры, либо используется неверный порядок использования параметров.\nУкажите параметр -h для вызова справки" << endl;
		return 0;
	}

	if (argc == 15 && (strcmp(argv[1], "-d") || strcmp(argv[3], "-u") || strcmp(argv[5], "-p") || strcmp(argv[7], "-b") || strcmp(argv[9], "-v") || strcmp(argv[11], "-c") || strcmp(argv[13], "-l")))
	{
		cout << "Указаны неверные параметры, либо используется неверный порядок использования параметров.\nУкажите параметр -h для вызова справки" << endl;
		return 0;
	}

	if (argc<13 || argc == 14 || argc > 15)
	{
		cout << "Неверное количество аргументов. Укажите параметр -h для вызова справки" << endl;
		return 0;
	}
	
	char* vmxPath = argv[2];
	char* login = argv[4];
	char* password = argv[6];
	char* browser = argv[8];
	char* version = argv[10];
	char* capacity = argv[12];

	if (strcmp(capacity, "32") && strcmp(capacity, "64"))
	{
		cout << "ОШИБКА! Параметр -c может иметь значение 32 или 64" << endl;
		return 0;
	}

	char* language = "en-US";

	if (argc == 15)
	{
		if (!strcmp(browser, "chrome"))
		{
			cout << "Параметр -l недоступен для браузера chrome" << endl;
			return 0;			
		}

		else
		{
			language = argv[14];
		}
	}


	

	char silent_install_options [150]= "";
	char website[100] = "";
	char path_to_browser_on_host[150] = "";
	char path_to_browser_on_virual_machine[150]="";
	char url_of_browser[400]="";
	char folder_name[100]="";
	char folder_on_vm[100] = "";
	char folder_with_installer_on_vm[100] = "";
	
	char buf_browser[20], buf_version[30], buf_capacity[20], buf_language[20];
	strcpy(buf_browser, browser);
	strcpy(buf_version, version);
	strcpy(buf_capacity, capacity);
	strcpy(buf_language, language);


	if (!strcmp(browser, "firefox"))
	{
		strcat(url_of_browser, "http://hicap.frontmotion.com.s3.amazonaws.com/Firefox/Firefox-");
		strcat(url_of_browser, version);
		strcat(url_of_browser, "/Firefox-");
		strcat(url_of_browser, version);
		strcat(url_of_browser, "-");
		strcat(url_of_browser, language);
		strcat(url_of_browser, ".msi");

		strcat(website, "http://www.frontmotion.com");

	}

	else if (!strcmp(browser, "chrome"))
	{
		if (strcmp(version, "last"))
		{
			cout << "Для браузера chrome доступна только версия last(последняя). Введите last в качестве версии" << endl;
			return 0;
		}
	}	

	else if (strcmp(browser, "opera"))
	{
		cout << "Браузер " << browser << " недоступен для установки.\nУкажите параметр -h для вызова справки" << endl;
		return 0;
	}


	if (!strcmp(browser, "firefox"))
	{

		strcat(folder_name, "c:\\");		
		strcat(folder_name, buf_browser);
		strcat(folder_name, "_");
		strcat(folder_name, buf_version);
	}

	else
	{
		strcat(folder_name, "c:\\Browsers");
	}

	strcat(path_to_browser_on_host, folder_name);	
	strcat(path_to_browser_on_host, "\\");
	strcat(path_to_browser_on_host, buf_browser);
	strcat(path_to_browser_on_host, "_");
	strcat(path_to_browser_on_host, buf_version);
	strcat(path_to_browser_on_host, "_");
	strcat(path_to_browser_on_host, buf_capacity);

	if (!strcmp(browser, "opera"))
	{
		strcat(path_to_browser_on_host, "_");
		strcat(path_to_browser_on_host, buf_language);
	}

	strcat(path_to_browser_on_host, ".msi");

	if (strcmp(browser, "firefox"))
	{
		ifstream fin(path_to_browser_on_host);

		if (!fin)
		{
			cout << "Данный браузер не доступен для установки"<<endl;
			fin.close();
			return 0;
		}

		fin.close();
	}
	

	strcat(folder_on_vm, "c:\\");
	strcat(folder_on_vm, buf_browser);
	strcat(folder_on_vm, "_");
	strcat(folder_on_vm, buf_version);
	strcat(folder_on_vm, "_");
	strcat(folder_on_vm, buf_capacity);

	strcat(folder_with_installer_on_vm, folder_on_vm);
	strcat(folder_with_installer_on_vm, "\\");
	strcat(folder_with_installer_on_vm, "installer");	
	
	strcat(path_to_browser_on_virual_machine, folder_with_installer_on_vm);
	strcat(path_to_browser_on_virual_machine, "\\");
	strcat(path_to_browser_on_virual_machine, buf_browser);
	strcat(path_to_browser_on_virual_machine, "_");
	strcat(path_to_browser_on_virual_machine, buf_version);
	strcat(path_to_browser_on_virual_machine, "_");
	strcat(path_to_browser_on_virual_machine, buf_capacity);
	strcat(path_to_browser_on_virual_machine, ".msi");

	strcat(silent_install_options, " /a ");
	strcat(silent_install_options, path_to_browser_on_virual_machine);
	strcat(silent_install_options, " TARGETDIR=");
	strcat(silent_install_options, folder_on_vm);
	strcat(silent_install_options, " /qn");	

	
	VixError err;
	int isLoggedIn = 0;
	VixHandle hostHandle = VIX_INVALID_HANDLE;
	VixHandle vmHandle = VIX_INVALID_HANDLE;
	VixHandle jobHandle = VIX_INVALID_HANDLE;

	if (!strcmp(browser, "firefox"))
	{
		_mkdir(folder_name);
	}

	CURL *curl;
	CURLcode res;
	struct FtpFile ftpfile = {
		path_to_browser_on_host,
		NULL
	};
	
	if (!strcmp(browser, "firefox"))
	{

		int download_code;
		curl_global_init(CURL_GLOBAL_DEFAULT);
		curl = curl_easy_init();

		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url_of_browser);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_file);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &download_code);
		}

		if (ftpfile.stream)
			fclose(ftpfile.stream);

		curl_global_cleanup();

		if (download_code != 200)
		{
			if (!strcmp(browser, "firefox") && strcmp(language, "en-US"))
				cout << "\nНа сайте " << website << " отсутствует браузер " << browser << " с версией " << version << " и языком " << language << endl;
			else
				cout << "\nНа сайте " << website << " отсутствует браузер " << browser << " с версией " << version << endl;
			goto abort;
		}

		cout << endl << "Инсталлятор браузера загружен из сети" << endl;	
	}	




	jobHandle = VixHost_Connect(VIX_API_VERSION, CONNTYPE, HOSTNAME, HOSTPORT, USERNAME, PASSWORD, 0, VIX_INVALID_HANDLE, NULL, NULL);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_JOB_RESULT_HANDLE, &hostHandle, VIX_PROPERTY_NONE);
	if (VIX_FAILED(err)) {
		cout << "ОШИБКА! Не удалось подключиться к хост-машине" << endl;
		goto abort;
	}
	Vix_ReleaseHandle(jobHandle);	
	
	jobHandle = VixVM_Open(hostHandle, vmxPath, NULL, NULL); 
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_JOB_RESULT_HANDLE, &vmHandle, VIX_PROPERTY_NONE); 
	if (VIX_FAILED(err)) {
		cout << "ОШИБКА! Виртуальная машина не найдена по указанному пути" << endl;
		goto abort;
	}
	Vix_ReleaseHandle(jobHandle);
	cout << "Виртуальная машина по указанному пути найдена" << endl;

	jobHandle = VixVM_PowerOn(vmHandle, VMPOWEROPTIONS, VIX_INVALID_HANDLE, NULL, NULL); 
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_FAILED(err)) {
		cout << "ОШИБКА! Не удалось включить виртуальную машину" << endl;
		goto abort;		
	}
	Vix_ReleaseHandle(jobHandle);
	cout << "Виртуальная машина включена" << endl;

	jobHandle = VixVM_WaitForToolsInGuest(vmHandle, 300, NULL, NULL);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
		cout << "ОШИБКА! VMware tools не установлены в гостевой операционной системе" << endl;
		goto abort;
	}
	Vix_ReleaseHandle(jobHandle);	

	jobHandle = VixVM_LoginInGuest(vmHandle, login, password, 0, NULL, NULL); 
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
		cout << "ОШИБКА! Неправильный логин или пароль" << endl;
		goto abort;		
	}
	isLoggedIn = 1;
	cout << "Аутентификация прошла успешно" << endl;
	Vix_ReleaseHandle(jobHandle);

	jobHandle = VixVM_CreateDirectoryInGuest(vmHandle, folder_on_vm, VIX_INVALID_HANDLE, NULL, NULL);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
	cout << "ОШИБКА! Не удается создать папку на виртуальной машине" << endl;
	goto abort;
	}
	Vix_ReleaseHandle(jobHandle);	

	jobHandle = VixVM_CreateDirectoryInGuest(vmHandle, folder_with_installer_on_vm, VIX_INVALID_HANDLE, NULL, NULL);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
		cout << "ОШИБКА! Не удается создать папку на виртуальной машине" << endl;
		goto abort;
	}
	Vix_ReleaseHandle(jobHandle);


	jobHandle = VixVM_CopyFileFromHostToGuest(vmHandle,	path_to_browser_on_host, path_to_browser_on_virual_machine, 0, VIX_INVALID_HANDLE, NULL, NULL); 	
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
		cout << "ОШИБКА! Не удалось переместить инсталлятор браузера с хост-машины на виртуальную машину" << endl;
		goto abort;
	}
	Sleep(20000);
	cout << "Инсталлятор браузера перемещен на виртуальную машину" << endl;
	Vix_ReleaseHandle(jobHandle);	

	jobHandle = VixVM_RunProgramInGuest(vmHandle, "msiexec.exe", silent_install_options, 0, VIX_INVALID_HANDLE, NULL, NULL);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
		cout << "ОШИБКА! Не удалось установить браузер" << endl;
		goto abort;
	}
	Sleep(20000);	

	cout << "Браузер успешно установлен на виртуальную машину" << endl;
	Vix_ReleaseHandle(jobHandle);
	

abort:

	if (!strcmp(browser, "firefox"))
	{
		remove(path_to_browser_on_host);
		_rmdir(folder_name);
	}

	if (isLoggedIn) {		
		Vix_ReleaseHandle(jobHandle);		
		jobHandle = VixVM_LogoutFromGuest(vmHandle,	NULL, NULL);
		err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
		if (VIX_OK != err) {			
		}
	}

	Vix_ReleaseHandle(jobHandle);
	Vix_ReleaseHandle(vmHandle);
	VixHost_Disconnect(hostHandle);

	return 0;
}