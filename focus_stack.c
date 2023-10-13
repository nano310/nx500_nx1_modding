/*
Compile with:

arm-linux-gnueabi-gcc -o focus_stack focus_stack.c -s `pkg-config --cflags --libs ecore elementary`  -lecore_input  -lpthread --sysroot=../arm/ -Wl,-dynamic-linker,/lib/ld-2.13.so

We need to specify the correct ld or it will not work on device.
*/
#define _GNU_SOURCE
#include <Elementary.h>
#include <strings.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>

#define SCREEN_WIDTH 720
#define MAX_STEPS 200
#define DEFAULT_STEPS 10

static int debug = 1, running = 0, popup_shown=0, entry_shown=0;
static int alpha_value = 255;
static char *settings_file = "/root/focus_stack.cfg";
static char *caption_near = "Near";
static char *caption_far = "Far";
static char *caption_conf= "Conf.";
static char *caption_start = "Start";
static char *version_model, *version_release;
static Evas_Object *entry_win,*lab, *win, *box, *btn_near, *btn_far, *btn_stack, *btn_quit, *btn_info, *entry_points, *entry_delay, *popup_box, *table, *btn_settings, *bg, *lab_2, *ok;
Evas_Object *popup_win;
Ecore_Timer *timer;
char stringline[255], label_entry[255], sample_text[255];
int focus_pos_near = 0, focus_pos_far = 0, focus_pos_min = 0, focus_pos_max =
    0, number_points = DEFAULT_STEPS, shot_delay = 6;
int button_height = 60, button_width = 120;
pthread_t timer_thread, cleanup_thread;

static void quit_app()
{
	elm_exit();
	exit(0);
}


long msec_passed(struct timeval *fromtime, struct timeval *totime)
{
  long msec;
  msec=(totime->tv_sec-fromtime->tv_sec)*1000;
  msec+=(totime->tv_usec-fromtime->tv_usec)/1000;
  return msec;
}

// Detect sleep events
void* timer_loop(void* arg) {
	struct timeval previous_time_loop, current_time_loop;
	int timer_sleep=1;
	long msec_elapsed;
	while(1) {
		gettimeofday(&previous_time_loop, NULL); 
		sleep(timer_sleep);
		gettimeofday(&current_time_loop, NULL); 
		msec_elapsed = msec_passed(&previous_time_loop,&current_time_loop);
		if (msec_elapsed>(timer_sleep*1000+300)) quit_app();
	}
}


static void run_command(char *command)
{
	if (debug) printf("CMD: %s\n", command);
	system(command);
}

int send_message(char * message_in){
	int i=0, result, fd=-1;
	unsigned char message_text[208]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	char  *param, *message, *spl;
	struct msg_buf_out {
		long mtype;
		char mtext[208];
	} msg;
	if (message_in[0]=='/')
		asprintf(&message,"%s",message_in);
	else
		asprintf(&message,"/usr/bin/st %s",message_in);
	
	spl=strtok(message," ");
	
	while(spl && strlen(spl)>0) {
		if (debug>2) printf("Adding at %d %s(%d)\n",4+i*20,spl,(int)strlen(spl));
		memcpy(message_text+4+i*20,spl,strlen(spl));
		i++;
		spl = strtok(NULL, " ");
	}
	asprintf(&spl,"%d",i);
	memcpy(message_text,spl, strlen(spl));

	if (debug>1) {
		printf("Message:\n");
		for(i=0;i<208;i++) {
			if ((i-4)%20==0) printf("\n");
			if (message_text[i]==0) 
				printf("_"); 
			else 
				printf("%c",(char)message_text[i]);
		}
		printf("\n");
	}

	if (fd<1) {
		fd = msgget(0x8828, 0666);
		if (fd<0) {
			perror(strerror(errno));
			printf("ERROR %d %d\n",errno, fd);
			return -1;
		}
	}
	msg.mtype=1;
	memcpy(msg.mtext, message_text, 208);
	result = msgsnd(fd, &msg, 208, 0);
	if (result<0) {
		perror( strerror(errno) );
		return -1;
	}
	return result;
}

static int version_load()
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen("/etc/version.info", "r");
	if (fp != NULL) {
		if ((read = getline(&line, &len, fp)) != -1) {
			line[strcspn(line, "\r\n")] = 0;
			asprintf(&version_release, "%s", line);
		}
		if ((read = getline(&line, &len, fp)) != -1) {
			line[strcspn(line, "\r\n")] = 0;
			asprintf(&version_model, "%s", line);
		}
		fclose(fp);
		free(line);
		return 0;
	}
	printf("Unable to determine device model and firmware version!\n");
	quit_app();
	return -1;
}

static void save_settings()
{
	FILE *f = fopen(settings_file, "w");
	if (f != NULL) {
		fprintf(f, "%d\n%d\n", number_points, shot_delay);
		fclose(f);
	}
}

static void settings_ok()
{
	evas_object_hide(entry_win);
	evas_object_show(win);
	int i = atoi(elm_object_text_get(entry_points));
	if (i > 0)
		number_points = i;
	i = atoi(elm_object_text_get(entry_delay));
	if (i > 0)
		shot_delay = i;
	char message[255];
	sprintf(message, "Frames: %d  delay: %ds",
		number_points, shot_delay);
// 	popup_show(message, 2, 3,1);
	if (debug) printf("Settings: %s\n", message);
	save_settings();
}


static void load_settings()
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(settings_file, "r");
	if (fp != NULL) {
		if (debug) printf("Reading configuration... ");
		if ((read = getline(&line, &len, fp)) != -1) {
			number_points = atoi(line);
		}
		if ((read = getline(&line, &len, fp)) != -1) {
			shot_delay = atoi(line);
		}
		fclose(fp);
		free(line);
		if (debug) printf ("%d %d\n",number_points, shot_delay);
	} else {
		printf ("Cannot access %s configuration file.\n",settings_file);
	}
}

static void click_quit(void *data, Evas_Object * obj, void *event_info)
{
	quit_app();
}

static Eina_Bool key_down_callback(void *data, int type, void *ev)
{
	Ecore_Event_Key *event = ev;
	if (debug) printf("Key: %s\n", event->key);
	if ((0 == strcmp("Super_R", event->key) ||
		0 == strcmp("Menu", event->key)||
		0 == strcmp("Super_L", event->key)))
	{
		elm_exit();
		exit(0);
	}
	
	if (0 == strcmp("Return", event->key) || 0 == strcmp("KP_Enter", event->key))
		settings_ok();
	
	if (0 == strcmp("XF86PowerOff", event->key)) {
		evas_object_hide(win);
		system("st key click pwoff");
	}
	return ECORE_CALLBACK_PASS_ON;
}


static Eina_Bool popup_hide()
{
	evas_object_hide(popup_win);
	evas_object_del(popup_win);
	popup_shown=0;
}

static Eina_Bool popup_timer_hide()
{
	popup_hide();
	return ECORE_CALLBACK_CANCEL;
}

void* force_update(void* arg) {
	ecore_main_loop_iterate();
	return NULL;
}

static void popup_show(char *message, int timeout, int row, int height)
{
	Evas_Object *popup_box, *lab, *table, *bg;
	char *command="";
	if (1==popup_shown) {
		if (debug) printf("Popup already shown!\n");
		return;
	}
		
	if (entry_win) {
// 		evas_object_hide(entry_win);
 		ecore_timer_del(timer);
	}// else {
		popup_win = elm_win_add(win, "Info", ELM_WIN_DIALOG_BASIC);
		elm_win_prop_focus_skip_set(popup_win, EINA_TRUE);
// 	}

	popup_box = elm_box_add(popup_win);
	elm_win_resize_object_add(popup_win, popup_box);
	evas_object_size_hint_min_set(popup_box, SCREEN_WIDTH, button_height*height);
	evas_object_show(popup_box);
	
	table = elm_table_add(popup_win);
	elm_box_pack_end(popup_box, table);
	evas_object_show(table);

	lab = elm_label_add(popup_win);
	evas_object_size_hint_min_set(lab, SCREEN_WIDTH, button_height*height);
	evas_object_show(lab);
	bg = evas_object_rectangle_add(evas_object_evas_get(lab));
	evas_object_size_hint_min_set(bg, SCREEN_WIDTH, button_height*height);
	evas_object_color_set(bg, 10, 30, 50, 255);
	evas_object_show(bg);
	elm_table_pack(table, bg, 1, 1, 1, 1);
	elm_table_pack(table, lab, 1, 1, 1, 1);
	evas_object_size_hint_min_set(lab, SCREEN_WIDTH, button_height*height);
	evas_object_size_hint_min_set(bg, SCREEN_WIDTH, button_height*height);
	elm_object_text_set(lab, message);
	evas_object_move(popup_win, 60, button_height * row);
	evas_object_show(popup_win);
	elm_win_render(popup_win);

	if (timeout > 0) {
		timer = ecore_timer_add(timeout, popup_timer_hide, NULL);
		popup_shown=1;
	}
	force_update(NULL);
}

static int get_af_position()
{
	FILE *fp;
	char *spl = NULL;
	fp = popen("/usr/bin/st cap iq af pos", "r");
	if (fp == NULL) {
		if (debug) printf("Failed get current focus position\n");
		return 0;
	} else {
		if (fgets(stringline, sizeof(stringline) - 1, fp) != NULL) {
			stringline[0] = '_';	// fix st output
			spl = strtok(stringline, " ");
			spl = strtok(NULL, " ");
			spl = strtok(NULL, " ");
		}
		pclose(fp);
		if (debug) printf("Current focus position: %s\n", spl);
		return atoi(spl);
	}
}

static unsigned int get_af_mode()
{
	FILE *fp;
	char *spl = NULL;
	fp = popen("/usr/bin/st cap capdtm getusr AFMODE", "r");
	if (fp == NULL) {
		if (debug) printf("Failed get current focus mode\n");
		return 0;
	} else {
		if (fgets(stringline, sizeof(stringline) - 1, fp) != NULL) {
			stringline[0] = '_';	// fix st output
			spl = strtok(stringline, "(");
			spl = strtok(NULL, ")");
		}
		pclose(fp);
		if (debug) printf("Current focus mode: %s (%ld)\n", spl,strtol(spl, NULL, 16));
		return strtol(spl, NULL, 16);
	}
}

static void set_af_mode(unsigned int af_mode)
{
	char * af_mode_s;
	asprintf(&af_mode_s,"0x%06x",af_mode);
	if (af_mode_s[strlen(af_mode_s)-1]=='0') run_command("/usr/bin/st app nx capture af-mode single");
	if (af_mode_s[strlen(af_mode_s)-1]=='1') run_command("/usr/bin/st app nx capture af-mode caf");
	if (af_mode_s[strlen(af_mode_s)-1]=='4') run_command("/usr/bin/st app nx capture af-mode auto");
	sprintf(stringline, "/usr/bin/st cap capdtm setusr AFMODE 0x%06x", af_mode);
	run_command(stringline);
}

static void focus_to_position(int position)
{
	int amount = 0;
	amount = position - get_af_position();
	sprintf(stringline, "/usr/bin/st cap iq af mv 255 %d 255", amount);
	//run_command(stringline);
	send_message(stringline);
}

static void focus_move(int amount)
{
	sprintf(stringline, "/usr/bin/st cap iq af mv 255 %d 255", amount);
	//run_command(stringline);
	send_message(stringline);
}

static void run_stack(int near, int far, int steps, int delay)
{
	int current_position = 0, step = 0;
	unsigned int af_mode=0;
	double delta = 0;
	char *stack_message="", *command="";
	if (debug)  printf("Stacking - Near: %d \tFar: %d \tPhotos: %d \tDelay: %d\n",
		   near, far, steps, delay);
	//Turn QuickView OFF
	if (0==strcmp("NX500",version_model)) {
		system("prefman set 0 0x0210 l 0;st cap capdtm setusr 51 0x0330000");
	}
	if (0==strcmp("NX1",version_model)) {
		system("prefman set 0 0x0210 l 0;st cap capdtm setusr 49 0x0310000");
	}

	af_mode = get_af_mode();
	run_command("/usr/bin/st app nx capture af-mode manual\n");	// show manual focus mode
	run_command("/usr/bin/st cap capdtm setusr AFMODE 0x70003\n");	// force manual focus mode
	sleep(1);
	focus_to_position(near);
	sleep(2);
	current_position = get_af_position();
	delta = ((double)(far - current_position)) / (double)(steps - 1);
	if (debug) printf("far: %d current: %d delta: %f\n", far, current_position, delta);
	sleep(delay / 2);
	while (step < steps && step < MAX_STEPS) {
		step++;
		asprintf(&stack_message, "#%d of %d",step,steps);
		if (strcmp("NX1",version_model)==0) {
			asprintf(&command,"/opt/usr/nx-on-wake/popup_timeout \"%s\" 1 &",stack_message);
			system(command);
		} else {
			popup_show(stack_message,1,0,1);
		}

		//run_command("/usr/bin/st app nx capture single\n");	// capture single frame
		send_message("app nx capture single");	// capture single frame
//         run_command("/usr/bin/st key push s1 && /bin/sleep 0.3 && /usr/bin/st key click s2 && /usr/bin/st key release s1 && /bin/sleep 0.5 && /usr/bin/st key click s1"); // capture single frame and exit photo preview is exists
		if (step == steps)
			break;
		sleep(delay);
		focus_move((int)
			   (near + (int)(step * delta) - current_position));
		current_position = near + (int)(step * delta);
	}
	set_af_mode(af_mode);
}

static void click_near(void *data, Evas_Object * obj, void *event_info)
{
	if (0==popup_shown) {
		popup_show("<align=center>Near focus position set.</align>", 2, 1,1);
		focus_pos_near = get_af_position();
	}
}

static void click_far(void *data, Evas_Object * obj, void *event_info)
{
	if (0==popup_shown) {
		popup_show("<align=center>Far focus position set.</align>", 2, 1,1);
		focus_pos_far = get_af_position();
	}
}

void * thread_stack(void *arg) {
	char *message;
	evas_object_hide(win);
	asprintf (&message, "<align=center>Making %d photos with delay %ds</align>",number_points,shot_delay);
	popup_show(message,1,0,1);
	running = 1;
	run_stack(focus_pos_near, focus_pos_far, number_points, shot_delay);
 	evas_object_hide(popup_win);
	evas_object_show(win);
	return (void *)0;
}

static void click_stack(void *data, Evas_Object * obj, void *event_info)
{
	if (0==popup_shown) {
		if (focus_pos_near == 0) {
			popup_show("Set NEAR focus point first!",2,1,1);
			return;
		}
		if (focus_pos_far == 0) {
			popup_show("Set FAR focus point first!",2,1,1);
			return;
		}
		pthread_t timer_thread;
		pthread_create(&timer_thread, NULL, &thread_stack, NULL);
	}
}

static void entry_show(int row)
{
	if (1==popup_shown) {
		if (debug) printf("Popup already shown!\n");
		return;
	}
	evas_object_hide(win);
	Evas_Object *lab, *lab_2, *bg, *popup_box, *table, *ok;
	if (entry_win) {
		evas_object_del(entry_win);
	}
	entry_win = elm_win_add(NULL, "Entry", ELM_WIN_BASIC);
	evas_object_move(entry_win, 30, button_height * row);
	bg = elm_bg_add(entry_win);
	elm_win_resize_object_add(entry_win, bg);
	evas_object_show(bg);
// 	evas_object_size_hint_min_set(entry_win, SCREEN_WIDTH, button_height);
// 	evas_object_size_hint_min_set(bg, SCREEN_WIDTH, button_height);

	popup_box = elm_box_add(entry_win);
// 	elm_box_horizontal_set(popup_box, EINA_TRUE);
	elm_win_resize_object_add(entry_win, popup_box);
	evas_object_show(popup_box);
	
	table = elm_table_add(entry_win);
	elm_box_pack_end(popup_box, table);

	lab = elm_label_add(entry_win);
	elm_object_text_set(lab, "Shots:");
	evas_object_show(lab);

	bg = evas_object_rectangle_add(evas_object_evas_get(lab));
	evas_object_color_set(bg, 0, 0, 0, 255);
	evas_object_size_hint_min_set(bg, 140, button_height);
	evas_object_show(bg);
	elm_table_pack(table, bg, 1, 1, 1, 1);

	elm_table_pack(table, lab, 1, 1, 1, 1);

	char *steps_string;
	asprintf(&steps_string, "%d", number_points);
	entry_points = elm_entry_add(entry_win);
	elm_entry_single_line_set(entry_points, EINA_TRUE);
	elm_entry_input_panel_layout_set(entry_points,ELM_INPUT_PANEL_LAYOUT_NUMBERONLY);
	elm_object_text_set(entry_points, steps_string);
	evas_object_show(entry_points);
	elm_entry_cursor_pos_set(entry_points, strlen(steps_string));

	bg = evas_object_rectangle_add(evas_object_evas_get(entry_points));
	evas_object_color_set(bg, 40, 40, 40, 255);
	evas_object_size_hint_min_set(bg, 140, button_height);
	evas_object_show(bg);
	elm_table_pack(table, bg, 2, 1, 1, 1);

	elm_table_pack(table, entry_points, 2, 1, 1, 1);

	lab_2 = elm_label_add(entry_win);
	elm_object_text_set(lab_2, "Delay:");
	
	bg = evas_object_rectangle_add(evas_object_evas_get(lab_2));
	evas_object_color_set(bg, 0, 0, 0, 255);
	evas_object_size_hint_min_set(bg, 140, button_height);
	evas_object_show(bg);
	elm_table_pack(table, bg, 3, 1, 1, 1);

	elm_table_pack(table, lab_2, 3, 1, 1, 1);
	evas_object_show(lab_2);

	char *delay_string;
	asprintf(&delay_string, "%d", shot_delay);
	entry_delay = elm_entry_add(entry_win);
	elm_entry_input_panel_layout_set(entry_delay,ELM_INPUT_PANEL_LAYOUT_NUMBERONLY);
	elm_entry_single_line_set(entry_delay, EINA_TRUE);
	elm_object_text_set(entry_delay, delay_string);
	elm_entry_cursor_pos_set(entry_delay, strlen(delay_string));
	evas_object_show(entry_delay);

	bg = evas_object_rectangle_add(evas_object_evas_get(entry_delay));
	evas_object_color_set(bg, 40, 40, 40, 255);
	evas_object_size_hint_min_set(bg, 140, button_height);
	evas_object_show(bg);
	elm_table_pack(table, bg, 4, 1, 1, 1);
	
	elm_table_pack(table, entry_delay, 4, 1, 1, 1);

	ok = elm_button_add(entry_win);
	elm_object_text_set(ok, "OK");
	elm_object_style_set(ok, "transparent");

	bg = evas_object_rectangle_add(evas_object_evas_get(ok));
	evas_object_color_set(bg, 0, 200, 0, 255);
	evas_object_size_hint_min_set(bg, 100, button_height);
	evas_object_show(bg);
	elm_table_pack(table, bg, 5, 1, 1, 1);

	elm_table_pack(table, ok, 5, 1, 1, 1);
	evas_object_show(ok);

	evas_object_show(table);
	evas_object_show(entry_win);
	elm_object_focus_set(entry_points, EINA_TRUE);

	evas_object_smart_callback_add(ok, "clicked", settings_ok, NULL);
	elm_win_render(entry_win);
}

static void click_settings(void *data, Evas_Object * obj, void *event_info)
{
	entry_show(1);
}

static void video_sweep() {
	int near, far;
	char *command;
	run_command("/usr/bin/st cap lens focus far");
	sleep(2);
	far = get_af_position();
	run_command("/usr/bin/st cap lens focus near");
	sleep(2);
	near = get_af_position();
	sleep(2);
	run_command("st key click del; sleep 1; st key click fn; sleep 1; /usr/bin/st cap lens focus near; sleep 1;st key click rec; sleep 0.5");
	asprintf(&command,"/usr/bin/st cap iq af mv 0 %d 0", (int)(far-near));
	run_command(command);
	run_command("sleep 2;st key click rec");
}

static void click_info(void *data, Evas_Object * obj, void *event_info)
{
	popup_show("focus_stack v2.30<br>Usage:<br>Focus on near point - click \"Near\"<br>\
				Focus on far point - click \"Far\"<br>Click \"Conf.\" to set number of photos<br>\
				Click on \"Start\" to start",10,1,5);
}


EAPI int elm_main(int argc, char **argv)
{
	// detect power-off and quit on it
	pthread_create(&cleanup_thread, NULL, &timer_loop, NULL);
	// determine model and version of camera
	version_load();
	if (argc > 1) {
		if (!strcmp(argv[1], "help")) {
			printf
			    ("Usage:\nfocus_stack [ help | sweep | /path/to/config_file | number_of_photos [ delay_between_photos [ button_height [ button_width ] ] ] ]\n\n");
			exit(0);
		}
		if (argv[1][0]=='/' || argv[1][0]=='.') {
			settings_file=argv[1];
			// load default settings
			load_settings();
		}
		else if (0==strcmp(argv[1], "sweep")) {
			video_sweep();
			exit(0);
		} else {
			if (atoi(argv[1])>0) 
				number_points = atoi(argv[1]);
		}
		if (argc > 2) {
			if (atoi(argv[2])>0) 
				shot_delay = atoi(argv[2]);
		}
		if (argc > 3) {
			if (atoi(argv[3])>0) 
				button_height = atoi(argv[3]);
		}
		if (argc > 4) {
			if (atoi(argv[4])>0) 
				button_width = atoi(argv[4]);
		}
	} else { 
		// load default settings
		load_settings();
	}

	printf("Stacking with %d photos at %d delay and %d button height\n",
	       number_points, shot_delay, button_height);

	win = elm_win_add(NULL, "Focus stacker", ELM_WIN_BASIC);
	elm_win_prop_focus_skip_set(win, EINA_TRUE);
	evas_object_move(win, 55, 0);
	evas_object_smart_callback_add(win, "delete,request", click_quit, NULL);

	box = elm_box_add(win);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_win_resize_object_add(win, box);
	evas_object_show(box);
	table = elm_table_add(win);
	elm_box_pack_end(box, table);

	evas_object_size_hint_min_set(box, SCREEN_WIDTH, button_height);

	btn_near = elm_button_add(win);
	elm_object_style_set(btn_near, "transparent");
	elm_object_text_set(btn_near, caption_near);
	evas_object_show(btn_near);
	evas_object_size_hint_min_set(btn_near, button_width, button_height);
	bg = evas_object_rectangle_add(evas_object_evas_get(btn_near));
	evas_object_size_hint_min_set(bg, button_width, button_height);
	evas_object_color_set(bg, 40, 120, 200, alpha_value);
	evas_object_show(bg);
	elm_table_pack(table, bg, 1, 1, 1, 1);
	elm_table_pack(table, btn_near, 1, 1, 1, 1);

	evas_object_smart_callback_add(btn_near, "clicked", click_near, NULL);

	btn_far = elm_button_add(win);
	elm_object_style_set(btn_far, "transparent");
	elm_object_text_set(btn_far, caption_far);
	evas_object_show(btn_far);
	evas_object_size_hint_min_set(btn_far, button_width, button_height);
	bg = evas_object_rectangle_add(evas_object_evas_get(btn_far));
	evas_object_size_hint_min_set(bg, button_width, button_height);
	evas_object_color_set(bg, 40, 80, 140, alpha_value);
	evas_object_show(bg);
	elm_table_pack(table, bg, 2, 1, 1, 1);
	elm_table_pack(table, btn_far, 2, 1, 1, 1);

	evas_object_smart_callback_add(btn_far, "clicked", click_far, NULL);

	btn_settings = elm_button_add(win);
	elm_object_style_set(btn_settings, "transparent");
	elm_object_text_set(btn_settings, caption_conf);
	evas_object_show(btn_settings);
	evas_object_size_hint_min_set(btn_settings, 120, button_height);
	bg = evas_object_rectangle_add(evas_object_evas_get(btn_settings));
	evas_object_size_hint_min_set(bg, 120, button_height);
	evas_object_color_set(bg, 220, 140, 00, alpha_value);
	evas_object_show(bg);
	elm_table_pack(table, bg, 6, 1, 1, 1);
	elm_table_pack(table, btn_settings, 6, 1, 1, 1);

	evas_object_smart_callback_add(btn_settings, "clicked", click_settings,
				       NULL);

	btn_stack = elm_button_add(win);
	elm_object_style_set(btn_stack, "transparent");
	elm_object_text_set(btn_stack, caption_start);
	evas_object_show(btn_stack);
	evas_object_size_hint_min_set(btn_stack, button_width, button_height);
	bg = evas_object_rectangle_add(evas_object_evas_get(btn_stack));
	evas_object_size_hint_min_set(bg, button_width, button_height);
	evas_object_color_set(bg, 0, 200, 0, alpha_value);
	evas_object_show(bg);
	elm_table_pack(table, bg, 7, 1, 1, 1);
	elm_table_pack(table, btn_stack, 7, 1, 1, 1);

	evas_object_smart_callback_add(btn_stack, "clicked", click_stack, NULL);

	btn_info = elm_button_add(win);
	elm_object_style_set(btn_info, "transparent");
	elm_object_text_set(btn_info, " i ");
	evas_object_show(btn_info);
	evas_object_size_hint_min_set(btn_info, 80, button_height);
	bg = evas_object_rectangle_add(evas_object_evas_get(btn_info));
	evas_object_size_hint_min_set(bg, 80, button_height);
	evas_object_color_set(bg, 0, 150, 150, alpha_value);
	evas_object_show(bg);
	elm_table_pack(table, bg, 8, 1, 1, 1);
	elm_table_pack(table, btn_info, 8, 1, 1, 1);

	evas_object_smart_callback_add(btn_info, "clicked", click_info, NULL);

	btn_quit = elm_button_add(win);
	elm_object_style_set(btn_quit, "transparent");
	elm_object_text_set(btn_quit, " X ");
	evas_object_show(btn_quit);
	evas_object_size_hint_min_set(btn_quit, 80, button_height);
	bg = evas_object_rectangle_add(evas_object_evas_get(btn_quit));
	evas_object_size_hint_min_set(bg, 80, button_height);
	evas_object_color_set(bg, 0, 0, 0, 255);
	evas_object_show(bg);
	elm_table_pack(table, bg, 9, 1, 1, 1);
	elm_table_pack(table, btn_quit, 9, 1, 1, 1);

	evas_object_smart_callback_add(btn_quit, "clicked", click_quit, NULL);

	evas_object_show(table);
	evas_object_show(win);

	ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, key_down_callback, NULL);
	elm_run();
	return 0;
}

ELM_MAIN()
