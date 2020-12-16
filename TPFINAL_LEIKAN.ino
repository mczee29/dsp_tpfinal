#include <Wire.h>
#include "MAX30100.h"
#include "PeakDetection.h"

#define N 100

//-----------DECLARACION DE VARIABLES Y OBJETOS------------------
MAX30100 sensor;
PeakDetection peakDetection;


int senial_o[N];
int senial_o_lp[N];
int senial_o_hp[N];
int senial_f[N];
int senial_diff[N];

int i=0;
int peak=0;
int peak_anterior=0;
int t_anterior=0;
int t_actual=0;
//-----------FIN DECLARACION DE VARIABLES Y OBJETOS------------------


//------------------- PASA BAJOS ------------------------------
//pyfda, butterworth, lowpass, orden 2, fs=100, fpb=10, psb=30, Asb=10
 float b0=0.2047;
 float b1=0.4094;
 float b2=0.2047;
 float a0=1.0;
 float a1=-0.3781;
 float a2=0.197;

int filtrar_pb(int original[],int filtrada[],int k){
  float acum=0;
  acum=b0*original[k]+b1*original[k-1]+b2*original[k-2]+a1*filtrada[k-1]+a2*filtrada[k-2];
  return int(acum);
}
//------------------FIN PASA BAJOS ----------------------------

//------------------ DIFERENCIAL ------------------------------
//pyfda, diff, fs=100, orden 3
float b0_d=-0.02;
float b1_d=0.57;
float b2_d=-0.57;
float b3_d=0.02;

int filtrar_diff(int original[],int filtrada[],int k){
  float acum=0;
  acum=b0_d*original[k]+b1_d*original[k-1]+b2_d*original[k-2]+b3_d*original[k-3];
  return int(acum);
}
//------------------ FIN DIFERENCIAL ------------------------------

//----------------------- PASA ALTOS --------------------------
//Pasa-altos, pyfda, orden 1, fc=0.3
float b0_hp=0.9695;
float b1_hp=-0.9695;
float a1_hp=0.9391;

int filtrar_hp(int original[],int filtrada[],int k){
  float acum=0;
  acum=b0_hp*original[k]+b1_hp*original[k-1]+a1_hp*filtrada[k-1];
  return int(acum);
}
//------------------FIN PASA ALTOS ----------------------------

void setup() {
  Serial.begin(115200);

  //inicializo los vectores
  senial_f[0]=1;
  senial_f[1]=1;
  senial_f[2]=1;
  senial_f[3]=1;
  senial_diff[0]=1;
  senial_diff[1]=1;
  senial_diff[2]=1;
  senial_diff[3]=1;
  senial_o_lp[0]=1;
  senial_o_lp[1]=1;
  senial_o_lp[2]=1;
  senial_o_hp[0]=1;
  senial_o_hp[1]=1;
  senial_o_hp[2]=1;
  
  //inicializo el sensor
  sensor.begin();

  //inicializo la detección de picos
  peakDetection.begin(50, 2, 0.5); // lag, threshold, influence
  
  delay(500);

  //configuracion del sensor
  sensor.setMode(MAX30100_MODE_SPO2_HR);
  delay(100);
  sensor.setLedsCurrent(MAX30100_LED_CURR_7_6MA, MAX30100_LED_CURR_7_6MA);
  delay(100);
  sensor.resetFifo();
}

void loop() {
  uint16_t ir, red;
  sensor.update();
  sensor.getRawValues(&ir, &red);
  senial_o[i]=ir;

  
  if (i>2){
    // señal original con pasa-altos, solo para graficar
    senial_o_hp[i]=filtrar_hp(senial_o, senial_o_hp, i);
    senial_o_lp[i]=filtrar_pb(senial_o_hp, senial_o_lp, i);
    
    // aplico el difErencial a la señal original y luego un pasabajos
    senial_diff[i]=filtrar_diff(senial_o, senial_diff, i);
    senial_f[i]=filtrar_pb(senial_diff, senial_f, i);

    //detección de picos
    peakDetection.add(-senial_f[i]); // la invierto a la señal
    peak = peakDetection.getPeak();


//-------------------- BLOQUE PRESENTACION LPM ------------------------
// comentar o descomentar este bloque si se desea graficar las señales
    if ((peak == 1) && (peak_anterior !=1)){
      t_actual = millis();
      float frec = 60.0*1000/(t_actual-t_anterior);
      if ((frec>50)&&(frec<210)){
        Serial.println(frec);  
      }
      else{
        Serial.println(999);  
      }
      t_anterior = t_actual;
    }
    peak_anterior=peak;
//------------------FIN BLOQUE PRESENTACION LMP -----------------------
    
  }

//---|||---||||------------ BLOQUE GRAFICACION SEÑALES ------||||------|||-----------
//comentar o descomentar un bloque para visualizar por serial plotter
//Solo un bloque de graficación a la vez activo

//----------------- SEÑAL ORIGINAL Y SEÑAL DIFERENCIAL ---------------  
  //Serial.print(-senial_o_lp[i]);
  //Serial.print(",");
  //Serial.println(-senial_f[i]*5);
//----------------- FIN SEÑAL ORIGINAL Y SEÑAL DIFERENCIAL ---------------  

//----------------- SEÑAL SEÑAL DIFERENCIAL Y DETECCION DE PICOS ---------------    
//  Serial.print(peak*20);
//  Serial.print(",");
//  Serial.println(-senial_f[i]);
//----------------- FIN SEÑAL SEÑAL DIFERENCIAL Y DETECCION DE PICOS ---------------    

//---|||---||||------------ FIN BLOQUE GRAFICACION SEÑALES ------||||------|||-----------
  delay(10);
  i++;
  
  // reseteo el buffer de las señales
  if(i>(N-1)){
    i=2;
    senial_f[0]=senial_f[N-3];
    senial_f[1]=senial_f[N-2];
    senial_f[2]=senial_f[N-1];
    senial_o[0]=senial_o[N-3];
    senial_o[1]=senial_o[N-2];
    senial_o[2]=senial_o[N-1];
    senial_diff[0]=senial_diff[N-3];
    senial_diff[1]=senial_diff[N-2];
    senial_diff[2]=senial_diff[N-1];
    senial_o_lp[0]=senial_o_lp[N-3];
    senial_o_lp[1]=senial_o_lp[N-2];
    senial_o_lp[2]=senial_o_lp[N-1];
    senial_o_hp[0]=senial_o_hp[N-3];
    senial_o_hp[1]=senial_o_hp[N-2];
    senial_o_hp[2]=senial_o_hp[N-1];
  }


}
