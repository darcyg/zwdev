# Modules for profuct:  
	* serial			: serial open/close/read/write  
	* transport 	: serial encapsulation(init, read, write, close)  
	* frame 			: frame parse and packet  
	* session			: frame queue function  
	* stateMachine: state machine module used for api module  
	* api 				: api state machine communicate with zwave module(zm5101)  
	* app 				: zwave static controller init and run  
	* cmd 				: cmd interface for debug  
	* mainLoop		: main loop io   
																-----------------------------  
																|         main loop         |  
																-----------------------------  
																|   app     |     cmd       |  
																-----------------------------  
																|   api     | state machine |	  
																-----------------------------  
																|  session  |  
																-------------  
																|   frame   |  
																-------------  
																| transport |  
																-------------  
																|  serial   |  
																-------------  
# serial:  
	/**
	 * @brief open serial device
	 * 
	 * @param dev serial device(such as /dev/ttyUSB0, /dev/ttyS1)
	 * @param buad open serial buadrate(such as 115200/9600/2400)
	 *
	 * @return 
	 * 			<= 0  fail
	 *			>  0  the opened serial file descriptor
	 */
	int serial_open(const char *dev, int buad);

	/** 
	 * @brief close serial device
	 *
	 * @param fd the descriptor to be closed
	 * 
	 * @return 
	 *			< 0  fail
	 *			= 0  ok
	 */
	int serial_close(int fd);

	/**
	 * @brief write data to serial
	 *
	 * @param fd the descroptor of serial device
	 * @param buf the data buffer to be wrotted
	 * @param size data size
	 * @param timeout_ms timeout for writtring
	 *
	 * @return
	 *			< 0 fail, error
	 * 			= 0 timeout
	 * 			> 0 success, the size of bytes wrotted
	 */
	int serial_write(int fd, char *buf, int size, int timeout_ms);

	/**
	 * @brief read data from serial
	 * 
	 * @param fd the descriptor of serial device
	 * @param buf the data buffer to store the data comming in.
	 * @param the buffer size
	 * @timeout_ms timeout for reading
	 * 
	 * @return
	 * 			< 0 fail, error
	 * 			= 0 timeout
	 *			> 0 success, the size of bytes readed
	 */
	int serial_read(int fd, char *buf, int size, int timeout_ms);

	/**
	 * @brief flush the serial line 
	 *
	 * @param fd the descriptor of serial device
	 *
	 * @return
	 * 			< 0 fail
	 *			= 0 ok
	 */
	int serial_flsuh(int fd);

# trasport:  
	/**
	 * @brief transport layer open
	 * 
	 * @param dev serial device 
	 * @param buadrate the buadrate 
	 * 
	 * @return
	 *			< 0 fail
	 *			= 0 ok
	 */
	int transport_open(const char *dev, int buadrate);
	/**
	 * @brief transport layer close
	 *
	 * @param none
	 * 
	 * @return 
	 *			< 0 fail
	 * 			= 0 ok
	 */
	int transport_close();

	/**
	 * @brief get file descriptor which transport layer using
	 * 
	 * @param none
	 * 
	 * @return
	 *			<= 0 fail
	 *			>  0 ok, the descriptor used by transport
	 */
	int transport_getfd();

	/**
	 * @brief check is transport initialized
	 *
	 * @param none
	 *
	 * @return 
	 *			0 not initialize
	 *			1 initialized
	 */
	int transport_is_open();

	/**
	 * @brief transport read data 
	 * 
	 * @param buf the buffer to store the data comming in.
	 * @param size the buffer size
	 * @param timeout_ms timeout time for reading
	 * 
	 * @return
	 *			< 0 fail
	 *			= 0 timeout
	 *			> 0 success, the readed bytes size
	 */
	int transport_read(char *buf, int size, int timeout_ms);

	/**
	 * @brief transport write data
	 * 
	 * @param buf the buf of data to be wroted
	 * @param size the data size
	 * @timeout_ms timeout time for writing
	 * 
	 * @return
	 *			< 0 fail
	 *			= 0 timeout
	 * 			> 0 success, the wrotted data size
	 */
	int transport_write(char *buf, int size, int timeout_ms);

# frame:  

	/**
	 * @brief frame layer init 
	 * 
	 * @param _th the timer header pointer used by frame layer to do timeout operation
	 * @param _send_over_cb callback function when completion of data sending
	 * @param _recv_over_cb callback function when completion of data receiving
	 * 
	 * @return
	 *			< 0 fail
	 *			= 0 ok
	 */
	int frame_init(void *_th, FRAME_SEND_OVER_CALLBACK _send_over_cb, FRAME_RECV_COMP_CALLBACK _recv_over_cb);
	
	/**
	 * @brief get the descriptor used by frame layer
	 *
	 * @param none
	 *
	 * @return 
	 *		 <= 0, fail
	 *			> 0, ok, the descriptor used by frame layer
	 */
	int frame_getfd();

	/**
	 * @brief reset the frame layer state 
	 *
	 * @param none
	 * 
	 * @return
	 *			0 ok
	 */
	int frame_receive_reset();

	/**
	 * @brief frame receive state step function
	 *
	 * @param none
	 *
	 * @return 
	 *			0 ok
	 */
	int frame_receive_step();

	/**
	 * @brief frame send 
	 * 
	 * @param df the frame to be sended
	 *
	 * @return 
	 * 			< 0 fail
	 *			= 0 ok
	 */
	int frame_send(stDataFrame_t *df);

	/**
	 * @brief frame layer uninit
	 *
	 * @param none
	 *
	 * @return 
	 *			0 ok
	 */
	int frame_free();

# session:  

	/**
	 * @brief session layer init
	 *
	 * @param none
	 * 
	 * @return 
	 *			0 ok
	 */
	int session_init(void *_th, SESSION_SEND_OVER_CALLBACK send_cb, SESSION_RECV_COMP_CALLBACK recv_cb);

	/**
	 * @brief session send frame
	 * 
	 * @param sf the frame to be sended
	 *
	 * @return 
	 * 			< 0 fail
	 * 			= 0 ok
	 */
	int session_send(void *sf);

	/**
	 * @brief get descriptor used by session
	 *
	 * @param none
	 *
	 * @return
	 *			<=0 fail
	 *			> 0 ok, the descriptor used by session
	 */
	int session_getfd();

	/**
	 * @brief session uninit
	 *
	 * @param none
	 * 
	 * @return 
	 *			0 ok
	 */
	int session_free();

	/**
	 * @brief session run step function
	 * 
	 * @param none
	 *
	 * @return 
	 *			0 ok
	 */
	int session_receive_step();

# stateMachine:  
	
	/**
	 * @brief state machine init function
	 *
	 * @param sm state machine 
	 *
	 * @return
	 *			< 0 fail
	 *		  = 0 ok
	 */
	int state_machine_init(stStateMachine_t *sm);

	/**
	 * @brief state machine reset function
	 *
	 * @param sm reset machine
	 *
	 * @return
	 * 			< 0 fail
	 *			= 0 ok
	 */
	 int state_machine_reset(stStateMachine_t *sm);

	/**
	 * @brief get state machine state
	 * 
	 * @param sm state machine
	 * 
	 * @return
	 *			- ok, the state of the state machine
	 */
	int state_machine_get_state(stStateMachine_t *sm);

	/**
	 * @brief state machine step function 
	 *
	 * @param sm state machine
	 * @param event the trigger event
	 *
	 * @return
	 *			- ok
	 */
	int state_machine_step(stStateMachine_t *sm, emStateEvent_t event);

	/**
	 * @brief state machine free function
	 *
	 * @param sm state machine 
	 */
	int state_machine_free(stStateMachine_t *sm);

# api:  

	/**
	 * @brief api init function 
	 *
	 * @param _th timer header used for api 
	 * @param _accb  api call callback function
	 * @param _arcb  api async data callback
	 */
	int api_init(void *_th, API_CALLBACK _accb, API_RETURN_CALLBACK _arcb);

	/**
	 * @brief api uninit 
	 *
	 * @param none
	 *
	 * @return 
	 *			< 0 fail
	 *			= 0 ok
	 */
	int api_free();

	/** 
	 * @brief api call function
	 *
	 * @param api api type
	 * @param param api argments
	 *
	 * @return 
	 *		< 0 fail
	 *		= 0 ok
	 */
	int api_call(emApi api, stParam *param);

	/**
	 * @brief get descriptor used by api layer
	 *
	 * @param none
	 * 
	 * @return
	 *		<= 0 fail
	 * 		>  0 ok, the descriptor used by api layer
	 */
	int api_getfd();

	/** 
	 * @brief api run step function
	 *
	 * @param none
	 *
	 * @return
	 *		0 ok
	 */
	int api_step();

	/**
	 * api state machine 
	 * /
	stStateMachine_t apism =  {
		2, 0, 0, {
			{S_IDLE, 3, NULL, {
					{E_FREE_RUNNING, idle_action_free_running, NULL},
					{E_CALL_API, idle_action_call_api, idle_transition_call_api},
					{E_ASYNC_DATA, idel_action_async_data, NULL},
				},
			},
			{S_RUNING, 3, NULL, {
					{E_ERROR, running_action_error, running_transition_error}, //nak can timeout
					{E_ACK, running_action_ack, running_transition_ack},
					{E_ASYNC_DATA, running_action_async, running_transition_async},
					{E_DATA, running_action_data, running_transition_data},
				},
			},
		},
	}

	/**
	 * api sub state machine
	 */
	stStateMachine_t getinitdata = {
		1, 0, 0, {
			{S_WAIT_INITDATA, 1, NULL, {
					{E_INITDATA, wait_initdata_action_initdata, wait_initdata_transition_initdata}
				},
			},
		},
	}

# app:  
	/**
	 * @brief app init function
	 * 
	 * @param none
	 * 
	 * @return
	 *		< 0 fail
	 * 		= 0 ok
	 */
	int app_init();
	
	/**
	 * @brief step app step function
	 * 
	 * @param none
	 *
	 * @return 
	 *			0 ok
	 */
	int app_step();

	/**
	 * @brief app get descriptor 
	 * 
	 * @param none
	 * 
	 * @return
	 *			< 0 fail
	 *			= 0 ok
	 */
	int app_getdf();

	/**
	 * @brief app uninit
	 *
	 * @param none
	 * 
	 * @return
	 *			0 ok
	 */
	int app_free();

# cmd:  
	/**
	 * @brief cmd init function
	 * 
	 * @param none
	 * 
	 * @return
	 *		< 0 fail
	 * 		= 0 ok
	 */
	int cmd_init();

	/**
	 * @brief cmd step function
	 * 
	 * @param none
	 *
	 * @return 
	 *			0 ok
	 */
	int cmd_step();

	/**
	 * @brief cmd get descriptor 
	 * 
	 * @param none
	 * 
	 * @return
	 *			< 0 fail
	 *			= 0 ok
	 */
	int cmd_getfd();

	/**
	 * @brief cmd uninit
	 *
	 * @param none
	 * 
	 * @return
	 *			0 ok
	 */
	int cmd_free();


# mainLoop:  
	main looper dispatcher .
