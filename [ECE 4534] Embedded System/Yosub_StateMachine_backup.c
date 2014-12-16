/* State Machine */
				if ( currentState == start ) {
					sprintf(lcdBuffer, "State: START");
					currentState = idle;
				}
				else if ( currentState == idle ) {
					sprintf(lcdBuffer, "State: IDLE");
					currentState = check_sensor;
				}
				else if ( currentState == check_sensor) {			
					// Check Front Sensor
					if ( FrontSensor < 65 ) {
							sprintf(lcdBuffer, "State: CHECK_SENSOR-ST");
							
							roverCommand[0] = i2cCommandST;
							if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
								VT_HANDLE_FATAL_ERROR(0);
							}
							currentState = stop;
						}
						else {
							sprintf(lcdBuffer, "State: CHECK_SENSOR-FW");
							currentState = check_sensor2;
						}
				}
				else if ( currentState == check_sensor2 ) {				
					sprintf(lcdBuffer, "State: CHECK_SENSOR2");
					
					/*
						4 Cases
						1. RFsensor < 40 then 	--> Parallel
						2. LFSensor < 35		--> TurnRight < 35
						3. RFsensor > 80 && RBsensor > 80 --> Stop2 --> TurnRightv
					*/
					currentState = moveFW;
					
					if ( LFsensor < 45 ) {
						currentState = stop2;
						break;
					}
					
					if ( RFsensor == 110 && RBsensor == 110 ) {
						currentState = stop3;
						break;
					}
					
					if ( RFsensor > 60 ) {
						currentState = turnRight_s;
						break;
					}
				}
				else if ( currentState == moveFW ) {
					sprintf(lcdBuffer, "State: moveFW");
					if ( roverState == 1 ) {
						roverCommand[0] = i2cCommandMF;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
								VT_HANDLE_FATAL_ERROR(0);
							}	
						currentState = parallel;
					}
				
				}
				else if ( currentState == parallel ) {
					sprintf(lcdBuffer, "State: PARALLEL");
					if ( roverState == 1 ) {
						int diff = RFsensor - RBsensor;
						currentState = idle;
						
						if ( diff > 2 && RFsensor < 65) {
							roverCommand[0] = i2cCommandTRS;
							currentState = parallel_wait;
						}
						
						if ( diff < -2 && RBsensor < 65) {
							roverCommand[0] = i2cCommandTLS;
							currentState = parallel_wait;
						}
						
						if ( RBsensor < 27 && RFsensor < 27 ) {
							roverCommand[0] = i2cCommandTLS;
							currentState = parallel_wait;
						}
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}
						
						sprintf(lcdBuffer, "State: PARALLEL -%d", diff);
					}
				}
				else if ( currentState == parallel_wait) {
					sprintf(lcdBuffer, "State: PARALLEL_WAIT");
					if ( counter > 2 ) {
						currentState = idle;
						counter = 0;
					}
					else {
						counter++;
					}
				}
				else if ( currentState == stop3 ) {
					sprintf(lcdBuffer, "State: STOP3");
					
					if ( counter > 10 ) {
						if (RFsensor == 110 && RBsensor == 110  ) {
							currentState = turnRight;
							counter = 0;
						}
						else {
							currentState = idle;
							counter = 0;
						}
					}
					else {
						counter ++;
					}
					
				}
				else if ( currentState == turnRight ) {
					sprintf(lcdBuffer, "Turn Right");
					
	
					if ( roverState == 1 ) {
						roverCommand[0] = i2cCommandTR;
						roverCommand[1] = 45;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}	
						currentState = wait_for_turn2;			
					}
					
				}
				else if ( currentState == wait_for_turn2) {
					sprintf(lcdBuffer, "State: wait_for_turn2");
					if ( counter > 15 ) {
						currentState = moveMF_after_right;
						counter = 0;
					}
					else {
						counter++;
					}
				}
				else if ( currentState == moveMF_after_right ) {
						sprintf(lcdBuffer, "State: moveMF_af_right");
						roverCommand[0] = i2cCommandMF;
						roverCommand[1] = 20;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}
						
						if ( RBsensor < 60 ) {
							currentState = idle;
						}
				}
				else if ( currentState == stop2 ) {
					sprintf(lcdBuffer, "State: STOP2");
					
					if ( counter > 10 ) {
						if ( LFsensor < 45 ) {
							currentState = turnRight_s;
							counter = 0;
						}
						else {
							currentState = idle;
							counter = 0;
						}
					}
					else {
						counter++;
					}
				}
				else if ( currentState == turnRight_s ) {
					sprintf(lcdBuffer, "State: TRS");
					if ( roverState == 1 ) {
						roverCommand[0] = i2cCommandTRS;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
								VT_HANDLE_FATAL_ERROR(0);
							}	
						currentState = idle;
					}
				}
				else if ( currentState == stop ) {
					sprintf(lcdBuffer, "State: STOP");
					if ( counter > 15 ) {
						if ( FrontSensor < 40 ){
							currentState = turnLeft;
						} else {
							roverCommand[0] = i2cCommandMF;
							roverCommand[1] = 20;
							if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
								VT_HANDLE_FATAL_ERROR(0);
							}
							currentState = idle;
						}
							counter = 0;
						
					}
					else {
						counter++;
					}
				
				}
				else if ( currentState == turnLeft ) {
					sprintf(lcdBuffer, "State: TURN LEFT");
					if ( roverState == 1 ) {
						roverCommand[0] = i2cCommandTL;
						roverCommand[1] = 45;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}	
						currentState = wait_for_turn;
					}
				}
				else if ( currentState == wait_for_turn) {
					sprintf(lcdBuffer, "State: WAIT_FOR_TURN");
					if ( counter > 15 ) {
						currentState = adjust;
						counter = 0;
					} 
					else {
						counter ++;
					}
					sprintf(lcdBuffer, "State: WAIT_TURN-%d", counter);
				}
				else if ( currentState == adjust ) {
					sprintf(lcdBuffer, "State: ADJUST");
					int diff = RFsensor - RBsensor;
					
					currentState = idle;
					
					if ( diff > 5 ) {
						sprintf(lcdBuffer, "State: ADJUST_R");
						roverCommand[0] = i2cCommandTRS;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}
						currentState = adjust_wait;
					}
					
					if ( diff < -5 ) {
						sprintf(lcdBuffer, "State: ADJUST_L");
						roverCommand[0] = i2cCommandTLS;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}	
						currentState = adjust_wait;
					}
				}
				else if ( currentState == adjust_wait ) {
					sprintf(lcdBuffer, "State: ADJUST_WAIT");
					if ( counter > 4 ) {
						currentState = adjust;
						counter = 0;
					}
					else {	
						counter++;
					}
				}

				// Send LCD Message
				if (SendLCDMsgStringDebug(lcdData, strnlen(lcdBuffer, vtLCDMaxLen), lcdBuffer, portMAX_DELAY) != pdTRUE) {
					VT_HANDLE_FATAL_ERROR(0);
				}
				

				FrontSensor_prev = FrontSensor;			
				RFsensor_prev = RFsensor;
				LFsensor_prev = LFsensor;
			