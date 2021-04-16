/*
 * ReceptionLabo2b.c
 *
 * Created: 2/12/2021 2:03:27 PM
 * Author : beaul
 */ 
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include "utils.h"
#include "driver.h"
#include "uart.h"
#include "fifo.h"

#include <math.h>;

#define MODE_SW1 1
#define	MODE_SW2 2
#define	MODE_SW3 3


const int VITESSE_MAX_ROUES_MODE_1 = 255;
const int VITESSE_MAX_ROTATION_MODE_1 = 100;
const int DIFFERENTIEL_MODE_1 = 25;

const int VITESSE_INERTIE = 100;

const int VALEUR_SERVO_ATTENTE = 1500;
const int VALEUR_SERVO_ARME = 2000;



//Elevateur
const int VITESSE_MAX_ELEVATEUR = 255;
	
double scaleAvantArriere(int entree, int vitesseMax);

double scaleDroiteGauche(int entree, int differentielMax);

double scaleServoMoteur(int entree, int vitesseMax);





int main(void)
{		

	

	//Valeur int recues de la manette
	int valeur_axe_x = 0;
	int valeur_axe_y = 0;
	int valeur_pot = 0;
	int valeur_sw1 = 0;
	int valeur_sw2 = 0;
	int valeur_sw3 = 0;
	int valeur_bp_joystick = 0;
	
	
	
	double vitesse;
	double differentiel;
	double vitesseRD;
	double vitesseRG;
	
	double vitesseElevateur = 0;
	double vitesseRoueInertie = 0;
	
	int valeur_servo = VALEUR_SERVO_ATTENTE;
	
	int state = 1;

	
	char string_recu[33];
	
	pwm0_init();
	pwm1_init(20000);
	pwm2_init();
	


	//Initialisation du module UART0
	uart_init(UART_0);
	sei();
	lcd_init();

	//INITILISATION DES SORTIES
	// A MODIFIER 
	//Mettre les led en sortie
	// Mettre les bits 0,1,2,3,4,5 du port des DELs en sortie
	//DDRB = set_bits(DDRB, 0b00011111);
	
	
	
	while (1)
	{
		
		valeur_axe_x = 0;
		valeur_axe_y = 0;
		valeur_pot = 0;
		valeur_sw1 = 0;
		valeur_sw2 = 0;
		valeur_sw3 = 0;
		valeur_bp_joystick = 0;
		vitesse = 0;
		vitesseRD = 0;
		vitesseRG= 0;
		differentiel= 0;
		vitesseElevateur = 0;
		vitesseRoueInertie = 0;
		valeur_servo = VALEUR_SERVO_ATTENTE;
		
		
		//Affcetation de la vitessem au moteur roues droites
		
		
		//Verifier l'etat de la file rx
		if (uart_rx_buffer_nb_line(UART_0)){
	
			
			//*********************************************
			//Recpetion du string recu par le transmetteur via UART
			//**********************************************
			uart_get_line(UART_0, string_recu, 17);
			//lcd_set_cursor_position(0,0);
			//lcd_write_string(string_recu);
			
			//********************************************
			//construction des valeurs numeriques recues du transmetteur
			//*******************************************
			valeur_axe_x += 100* char_to_uint(string_recu[1]) +  10* char_to_uint(string_recu[2])+  char_to_uint(string_recu[3]);
			valeur_axe_y += 100* char_to_uint(string_recu[4]) +  10* char_to_uint(string_recu[5])+  char_to_uint(string_recu[6]);
			valeur_pot += 100* char_to_uint(string_recu[7]) +  10* char_to_uint(string_recu[8])+  char_to_uint(string_recu[9]);
			valeur_sw1 = string_recu[10]- 48;
			valeur_sw2 = string_recu[11]- 48;
			valeur_sw3 = string_recu[12]- 48;
			valeur_bp_joystick = string_recu[13]- 48;
			
			//********************************************
			// TEST AFFICHAGE CONSTRUCTION VALEUR
			//********************************************
			char str_test [32];
			//sprintf(str_test,"%3d%3d%3d%d%d%d%d", valeur_axe_x , valeur_axe_y, valeur_pot, valeur_sw1, valeur_sw2, valeur_sw3, valeur_bp_joystick);
			//lcd_set_cursor_position(0,1);
			//lcd_write_string(str_test);
			
			//********************************************
			// Gestion de la machine d'�tat selon le mode
			//********************************************
			if (valeur_sw1 == 0){
				state = MODE_SW1;
				lcd_clear_display();
			}
			if (valeur_sw2 == 0){
				state = MODE_SW2;
				lcd_clear_display();
			}
			if (valeur_sw3 == 0){
				state = MODE_SW3;
				lcd_clear_display();
			}
			//********************************************
			// AFFECTATION DES SORTIES SELON LE MODE
			//********************************************
			
			switch(state){
				//MODE NORMAL 
				case MODE_SW1:
				lcd_set_cursor_position(0,0);
				lcd_write_string("Mode normal  ");
	
				
				//Scaling de la vitesse
				vitesse = scaleAvantArriere(valeur_axe_x,VITESSE_MAX_ROUES_MODE_1);
				differentiel = scaleDroiteGauche(valeur_axe_y, DIFFERENTIEL_MODE_1);	
				
			
				
				
				if (vitesse > 0){
					vitesseRD = vitesse - differentiel;
					vitesseRG = vitesse + differentiel;
					//Roues gauches U7 et U9-----------------------------
					//Affectation de la bonne direction sur les pont en H respectif
					PORTB = set_bit(PORTB, PB1);	
					//Roues droites U6 et U8------------------------------
					//Affectation de la bonne durection sur les pont en H respectif
					PORTB = clear_bit(PORTB, PB2);
				}
				if (vitesse < 0){
						vitesseRD = vitesse + differentiel;
						vitesseRG = vitesse - differentiel;
					//Roues gauches U7 et U9-----------------------------
					//Affectation de la bonne direction sur les pont en H respectif
					PORTB = clear_bit(PORTB, PB1);
					//Roues droites U6 et U8------------------------------
					//Affectation de la bonne durection sur les pont en H respectif
					PORTB = set_bit(PORTB, PB2);
					
				}
				
				/////////////////////
				//EDGE CASES !
				///////////////////
				
		
				if (vitesseRD > VITESSE_MAX_ROUES_MODE_1){
					vitesseRD = VITESSE_MAX_ROUES_MODE_1;
				}
				if (vitesseRG > VITESSE_MAX_ROUES_MODE_1){
					vitesseRG = VITESSE_MAX_ROUES_MODE_1;
				}
				if (vitesseRD < -VITESSE_MAX_ROUES_MODE_1){
					vitesseRD = -VITESSE_MAX_ROUES_MODE_1;
				}
				if (vitesseRG < -VITESSE_MAX_ROUES_MODE_1){
					vitesseRG = -VITESSE_MAX_ROUES_MODE_1;
				}
				
				
				
				else if (valeur_axe_x < 145 && valeur_axe_x > 110){
					vitesse = scaleDroiteGauche(valeur_axe_y, VITESSE_MAX_ROTATION_MODE_1);
					vitesseRG = vitesse;
					vitesseRD = vitesse;
					if (vitesse > 0){
						//Roues gauches U7 et U9-----------------------------
						//Affectation de la bonne direction sur les pont en H respectif
						PORTB = set_bit(PORTB, PB1);
						//Roues droites U6 et U8------------------------------
						//Affectation de la bonne durection sur les pont en H respectif
						PORTB = set_bit(PORTB, PB2);
						
					}
					else if (vitesse < 0){
						//Roues gauches U7 et U9-----------------------------
						//Affectation de la bonne direction sur les pont en H respectif
						PORTB = clear_bit(PORTB, PB1);
						//Roues droites U6 et U8------------------------------
						//Affectation de la bonne durection sur les pont en H respectif
						PORTB = clear_bit(PORTB, PB2);
						
					}
				}
				
				
			///////////////////////////////////////////////////
			//				    ELEVATEUR
			///////////////////////////////////////////////////
			
			//1 Calcul vitesse moteur
			
			vitesseElevateur = scaleServoMoteur(valeur_pot, VITESSE_MAX_ELEVATEUR);
			
			
			//2. sens de rotation
			//Bon sens de retotion de lelevateur
			
			if (vitesseElevateur > 0){
				PORTB = set_bit(PORTB, PB0);
			}
			if (vitesseElevateur < 0){
				PORTB = clear_bit(PORTB, PB0);
			}
			
			
			//3. Edge Cases
			
			//Ajutement des valeurs hors du range
			if (vitesseElevateur > VITESSE_MAX_ELEVATEUR){
				vitesseElevateur = VITESSE_MAX_ELEVATEUR;
			}
			if (vitesseElevateur < -VITESSE_MAX_ELEVATEUR){
				vitesseElevateur = -VITESSE_MAX_ELEVATEUR;
			}	
			
			
			///////////////////////////////////////////////////
			//				SERVO MOTEUR
			///////////////////////////////////////////
			//Verifiction de la valeur du jystick pour actionner le servomoteur si besoin
			if (valeur_bp_joystick == 0){
				//Actionner le sevo moteur
				valeur_servo = VALEUR_SERVO_ARME;
			}		
				
				break;

			}
			
			vitesseRG = abs(vitesseRG);
			vitesseRD = abs(vitesseRD);
			
			
			//Affcetation de la vitessem au moteur roues gauches
			pwm0_set_PB3(vitesseRG);
			//Affcetation de la vitessem au moteur roues droites
			pwm0_set_PB4(vitesseRD);
			
			
			//Elevateur 
		
			vitesseElevateur = abs(vitesseElevateur);
			pwm2_set_PD6(vitesseElevateur);
			
			//roue inertie
			pwm2_set_PD7(VITESSE_INERTIE);
			
			//------------------------------------------------------
			//servomoteur
			pwm1_set_PD5(valeur_servo);
			
			//Affichage sur le LCD
			char strRD[10];
			char strRG[10];
			char strelev[10];
			uint8_to_string(strRG, vitesseRG);
			uint8_to_string(strRD, vitesseRD);
			uint8_to_string(strelev, vitesseElevateur);
			
			lcd_set_cursor_position(0, 1);
			lcd_write_string("RG:");
			lcd_set_cursor_position(3, 1);
			lcd_write_string(strRG);
			lcd_set_cursor_position(6, 1);
			lcd_write_string("RD:");
			lcd_set_cursor_position(9, 1);
			lcd_write_string(strRD);
			lcd_set_cursor_position(12, 1);
			lcd_write_string(strelev);
			
			
			
			

		}
		
					
			
		}
		
	
}



double scaleAvantArriere(int entree, int vitesseMax){
	//On commence par le centrer sur 0 
	double deltay = vitesseMax;
	double deltax = vitesseMax-123;
	double y = deltay/deltax * (entree - 123);
	double a;
	double vitesse;
	//on le change pour une fonction de degre 3
	a = vitesseMax/pow(vitesseMax+100, 3);
	
	if (y>0){
		
		vitesse = a*pow(y+100,3);
	}
	if (y< 0){
		
		vitesse = a*pow(y-100,3);
	}
	
	
	return vitesse;
}

double scaleServoMoteur(int entree, int vitesseMax){
	//On commence par le centrer sur 0
	double deltay = vitesseMax;
	double deltax = vitesseMax-123;
	double y = deltay/deltax * (entree - 123);
	double a;
	double vitesse;
	//on le change pour une fonction de degre 3
	a = vitesseMax/pow(vitesseMax+100, 3);
	if (y>0){
		vitesse = a*pow(y+100,3);
	}
	if (y < 0){
		vitesse = a*pow(y-100,3);
	}
	
	if (entree < 155 && entree > 100 ){
		vitesse = 0;
	}
	
	return vitesse;
}



double scaleDroiteGauche(int entree, int differentielMax){
	double deltay = differentielMax*2;
	double deltax = 255;
	double y = deltay/deltax * (entree - 130);
	return y;
}
