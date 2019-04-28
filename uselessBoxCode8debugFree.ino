 
#include <Servo.h>
#define boxServoPin 5
#define armServoPin 6
#define switchPin 2

// ----------------------------------------------------------------------------------------------- Constants
#define angryMax 12
#define timer0 40 //50
#define timer1 100
#define servoDelayBox 15
#define servoDelayArm 8

//arm constants
#define armBackAngle 177
#define armFrontAngle 15
#define armTouch0 28
#define armTouch1 45
#define armSpeed0 2
#define armSpeed1 8

//box constants
#define boxCloseAngle 20
#define boxPeekAngle 42
#define boxOpenAngle 62
#define boxSpeed0 3
#define boxSpeed1 8

// ----------------------------------------------------------------------------------------------- STATES
#define S_stb        0
#define S_ready      1
#define S_opening    2
#define S_wo         3 //waiting open
#define S_closing    4
#define S_closingp   5 //closing peek
#define S_fronting   6
#define S_touching   7
#define S_waitingt   8
#define S_backing    9
#define S_wc        11

// ----------------------------------------------------------------------------------------------- EVENTS
#define E_0     0
#define E_BO    1 //box open
#define E_BC    2 //box closed
#define E_BP    3 //box peek
#define E_U     4 //switch up
#define E_D     5 //switch down
#define E_AB    6 //arm back
#define E_T0    7 //timer0
#define E_T1    8 //timer1
#define E_AT0   9//arm touch 0
#define E_AT1   10//arm touch 1
#define nEvents 11


// ----------------------------------------------------------------------------------------------- ACTIONS
#define A_0      0
#define A_AF     1 // set arm moving foward
#define A_AB     2 // set arm moving backwards
#define A_AS     3 // stop arm
#define A_BO     4 //set box opening
#define A_BC     5 //set box closing
#define A_BS     6 //stop box
#define A_AS0    7 //arm speed 0
#define A_AS1    8 //arm speed 1
#define A_BS0    10 //box speed 0
#define A_BS1    11 //box speed 1
#define A_TG     13 //timer go
#define A_TS     14 //timer stop
#define A_A      15 //more angry
#define A_SETUP  16
#define A_C      17 //crazy
#define A_A_     18
#define A_BSR    19
#define A_ASR    20
#define nActions 21


// ----------------------------------------------------------------------------------------------- Variables declarations
Servo box;
Servo arm;

byte state;
bool events[nEvents]; //events[E_i]==true if E_i is happening 
byte actionsToDo[nActions];
byte r;
byte n;
byte nd;
byte j;
byte r01=random(1,4);
byte r02=random(1,3);;

bool armMovingFoward;
bool armMovingBackwards;
byte armSpeed;
bool boxOpening=false;
bool boxClosing=false;
byte boxSpeed=false;

int counter=0;
byte angry=0;

bool counting=false;
bool time0=false;
bool time1=false;
int counter2=0;
// ----------------------------------------------------------------------------------------------- setup
void setup() {
  randomSeed(analogRead(A0));
  //servos
  box.attach(boxServoPin);
  box.write(boxCloseAngle);
  arm.attach(armServoPin);
  arm.write(armBackAngle);
  //digital input pin
  pinMode(switchPin, INPUT);

  clearActions();
  resetEvents();

  state=S_stb;
  doAction(A_TS);
  doAction(A_BS);
  doAction(A_AS);
}

// ----------------------------------------------------------------------------------------------- loop
void loop() {
  moveAll();      //moves servos and counters
  updateEvents(); //update array events[] so that events[E_i] is true if E_i has happned
  
  updateStateAndActions(); //changes state based on events[] and current state
                           //adds actions to actions[]
  
  for(byte i=0; i<nActions; i=i+1){
    if (actionsToDo[i]!=A_0){
      doAction(actionsToDo[i]);//does action
      actionsToDo[i]=A_0;      //resets list
    }
    else{
      break;                  //if a first A_0 is found, the rest of the list is A_0
    }
  }

}




// ----------------------------------------------------------------------------------------------- State Machine Functions
void updateStateAndActions(){
  //if (state==S_){
  //  if(events[E_]==true and r==){
  //    state=S_;
  //    actionsToDo[0]=A_;
  //    actionsToDo[1]=A_;
  //    return;
  //  }
  // }

  //waiting closed
  if (state==S_wc){
    if(n==0){
      state=S_opening;
      actionsToDo[0]=A_TS;
      actionsToDo[1]=A_BSR;
      actionsToDo[2]=A_BO;
      r=1;
      return;
    }
    if(events[E_T0]==true and n!=0){
      state=S_wc;
      n=n-1;
      actionsToDo[0]=A_TG;
      return;
    }
   }

  //stb
  if (state==S_stb){
    if(events[E_U]==true and angry<angryMax){
      state=S_ready;
      actionsToDo[0]=A_A;
      r=0;
      return;
    }
    if(angry==angryMax){
      state=S_ready;
      actionsToDo[0]=A_C;
      r=0;
      return;
    }
   }

  //ready
  if (state==S_ready){
    if(events[E_U]==true){
      state=S_opening;
      while(r==0 or r==r01){
      r=random(1,4);
      //r01
      if(angry>=angryMax/2){if(random(1,4)==1){r=4;}}
      }
      if(r==2){actionsToDo[0]=A_BS0;}
      else{actionsToDo[0]=A_BSR;}
      actionsToDo[1]=A_BO;
      r01=r;
      return;
    }
   }

  //opening
  if (state==S_opening){
    if(events[E_D]==true){
      state=S_closing;
      r=0;
      actionsToDo[0]=A_BS1;
      actionsToDo[1]=A_BC;
      actionsToDo[2]=A_A;
      return;
    }
    if(events[E_BO]==true and events[E_U]==true and r==3){
      state=S_fronting;
      r=0;
      actionsToDo[0]=A_BS;
      actionsToDo[1]=A_ASR;
      actionsToDo[2]=A_AF;
      return;
    }
    if(events[E_BO]==true and events[E_U]==true and r==4){
      state=S_fronting;
      r=2;
      actionsToDo[0]=A_BS;
      actionsToDo[1]=A_AS1;
      actionsToDo[2]=A_AF;
      return;
    }
    if(events[E_BP]==true and events[E_U]==true and r==2){
      state=S_wo;
      actionsToDo[0]=A_TG;
      actionsToDo[1]=A_BS;
      n=random(1,3);
      if (n==1){n=1;}
      if (n==2){n=2;}
      r=0;
      return;
    }
    if(events[E_BO]==true and events[E_U]==true and r==1){
      state=S_wo;
      n=random(1,3);
      if (n==1){n=1;}
      if (n==2){n=2;}
      r=0;
      actionsToDo[0]=A_BS;
      actionsToDo[1]=A_TG;
      return;
    }
    
  }

  //waiting open
  if (state==S_wo){
    if(events[E_D]==true){
      state=S_closing;
      actionsToDo[0]=A_BS1;
      actionsToDo[1]=A_BC;
      actionsToDo[2]=A_A;
      actionsToDo[3]=A_TS;
      return;
    }
    if(events[E_T0]==true and n!=0){
      state=S_wo;
      n=n-1;
      actionsToDo[0]=A_TG;
      return;
    }
    if(events[E_U]==true and events[E_BP]==false and n==0){
      state=S_fronting;
      actionsToDo[0]=A_TS;
      actionsToDo[1]=A_ASR;
      actionsToDo[2]=A_AF;
      return;
    }
    if(events[E_U]==true and events[E_BP]==true and n==0){
      state=S_closingp;
      actionsToDo[0]=A_TS;
      actionsToDo[1]=A_BS1;
      actionsToDo[2]=A_BC;
      return;
    }
   }

  //closing
  if (state==S_closing){
    if(events[E_BC]==true){
      state=S_stb;
      actionsToDo[0]=A_BS;
      return;
    }
    if(events[E_U]==true and angry<angryMax){
      state=S_opening;
      actionsToDo[0]=A_BS1;
      actionsToDo[1]=A_BO;
      actionsToDo[2]=A_A;
      r=4;
      return;
    }
   }

  //closing peek
  if (state==S_closingp){
    if(events[E_D]==true){
      state=S_closing;
      actionsToDo[0]=A_BS1;
      actionsToDo[1]=A_BC;
      actionsToDo[2]=A_A;
      r=1;
      return;
    }
    if(events[E_BC]==true and events[E_U]==true){
      state=S_opening;
      actionsToDo[0]=A_BS;
      actionsToDo[1]=A_BSR;
      actionsToDo[2]=A_BO;
      r=1;
      return;
    }
   }

  //fronting
  if (state==S_fronting){
    if(events[E_D]==true){
      state=S_backing;
      actionsToDo[0]=A_AS;
      actionsToDo[1]=A_AS1;
      actionsToDo[2]=A_AB;
      return;
    }
    if(r==0){
      state=S_fronting;
      while(r==0 or r==r02){
      r=random(1,3);
      //r02
      if(random(1,3)==2){r=2;}
      }
      r02=r;
      return;
    }
    if(events[E_D]==true and r==2){
      state=S_backing;
      actionsToDo[0]=A_AS;
      actionsToDo[1]=A_ASR;
      actionsToDo[2]=A_AB;
      return;
    }
    if(events[E_AT1]==true and r==1){
      state=S_touching;
      r=random(1,6);
      actionsToDo[0]=A_AS0;
      n=1;
      return;
    }
   }


  //backing
  if (state==S_backing){
    if(events[E_U]==true and angry<angryMax){
      state=S_fronting;
      r=2;
      actionsToDo[0]=A_AS1;
      actionsToDo[1]=A_AF;
      actionsToDo[2]=A_A;
      return;
    }
    if(angry==angryMax and events[E_U]==true and not events[E_AB]==true){
      state=S_backing;
      actionsToDo[0]=A_AS1;
      actionsToDo[1]=A_A;
      return;
    }
    if(angry==angryMax and events[E_U]==true and events[E_AB]==true){
      state=S_closing;
      actionsToDo[0]=A_AS;
      actionsToDo[1]=A_BS1;
      actionsToDo[2]=A_BC;
      return;
    }
    if(events[E_AB]==true){
      state=S_closing;
      actionsToDo[0]=A_AS;
      actionsToDo[1]=A_BS1;
      actionsToDo[2]=A_BC;
      return;
    }
   }

  //touching
  if (state==S_touching){
    if(events[E_D]==true){
      state=S_backing;
      actionsToDo[0]=A_AS1;
      actionsToDo[1]=A_AB;
      return;
    }
    if(r==0){
      state=S_fronting;
      actionsToDo[0]=A_AF;
      actionsToDo[1]=A_AS1;
      r=2;
      n=0;
      return;
    }
    if(r!=0 and (events[E_AT0]==true and n==1)){
      state=S_waitingt;
      actionsToDo[0]=A_AS;
      actionsToDo[1]=A_TG;
      return;
    }
    if(r!=0 and events[E_AT1]==true and n==0){
      state=S_waitingt;
      actionsToDo[0]=A_AS;
      actionsToDo[1]=A_TG;
      return;
    }
   }

  //waiting touch
  if (state==S_waitingt){
    if(events[E_D]==true){
      state=S_backing;
      actionsToDo[0]=A_AS1;
      actionsToDo[1]=A_AB;
      return;
    }
    if(events[E_T0]==true and n==1){
      state=S_touching;
      r=r-1;
      actionsToDo[0]=A_AB;
      actionsToDo[1]=A_TS;
      n=0;
      return;
    }
    if(events[E_T0]==true and n==0){
      state=S_touching;
      r=r-1;
      actionsToDo[0]=A_AF;
      actionsToDo[1]=A_TS;
      n=1;
      return;
    }
   }

  
  
}
// ------------------------------------------------------------------------------------------ update events 
void updateEvents(){
  events[E_BO]=BO();
  events[E_BC]=BC();
  events[E_BP]=BP();
  events[E_AB]=AB();
  events[E_T0]=T0();
  events[E_T1]=T1();
  events[E_AT0]=AT0();
  events[E_AT1]=AT1();

  //avoid noise
  nd=0;
  for(int i=0; i<=20; i++){
    if(D()){nd++;}
    delay(1);
  }
  if (nd>=19){events[E_D]=true;events[E_U]=false;}
  else{events[E_D]=false;events[E_U]=true;}
  
}

// ------------------------------------------------------------------------------------------ Do Action
void doAction(byte actionIndex){
  switch (actionIndex){
    case A_AF:
      armMovingFoward=true;
      armMovingBackwards=false;
      break;
    case A_AB:
      armMovingBackwards=true;
      armMovingFoward=false;
      break;
    case A_AS:
      armMovingFoward=false;
      armMovingBackwards=false;
      break;
    case A_BO:
      boxOpening=true;
      boxClosing=false;
      break;
    case A_BC:
      boxOpening=false;
      boxClosing=true;
      break;
    case A_BS:
      boxOpening=false;
      boxClosing=false;
      break;
    case A_AS0:
      armSpeed=armSpeed0;
      break;
    case A_AS1:
      armSpeed=armSpeed1;
      break;
    case A_BS0:
      boxSpeed=boxSpeed0;
      break;
    case A_BS1:
      boxSpeed=boxSpeed1;
      break;
    case A_BSR:
      if(random(0,2)==0){
        boxSpeed=boxSpeed0;
      }
      else{
        boxSpeed=boxSpeed1;
      }
      break;
        case A_ASR:
      if(random(0,2)==0){
        armSpeed=armSpeed0;
      }
      else{
        armSpeed=armSpeed1;
      }
      break;
    case A_TG:
      counter2=0;
      time0=false;
      time1=false;
      counting=true;
      break;
   case A_TS:
      counting=false;
      time0=false;
      time1=false;
      break;
   case A_A:
      moreAngry();
      break;
   case A_A_:
      angry=2;
      break;
   case A_C:
      boxSpeed=boxSpeed1;
      box.write(boxOpenAngle);
      delay(random(500,1000));
      j=random(1,3);
      for (byte i=0; i<j; i++){
        delay(500);
        n=random(3,6);
        while(n>0){
          box.write(boxCloseAngle);
          delay(120);
          box.write(boxPeekAngle);
          delay(100);
          n=n-1;
        }
        box.write(boxOpenAngle);
        delay(random(500,1000));    
      }
      box.write(boxCloseAngle);
      delay(2000);
      doAction(A_A_);
      break;
    
  }
}

// ----------------------------------------------------------------------------------------------- hardware and auxiliary functions
void clearActions(){
  for(byte i=0; i<nActions; i=i+1){
    actionsToDo[i]=A_0;
  }
}

void resetEvents(){
 for(byte i=0; i<nEvents; i=i+1){
    events[i]=false;
  }
}

void moreAngry(){
  angry++;
  if (angry>angryMax){
    angry=angryMax;
  }
  counter=0;
}

//MOVE BOX
void boxOpen(byte stp){
  if (box.read()+stp>boxOpenAngle){
    box.write(boxOpenAngle);
  }
  else{
    box.write(box.read()+stp);
  }
  delay(servoDelayBox);
}
void boxClose(byte stp){
  if (box.read()-stp<(boxPeekAngle)){
    box.write(boxCloseAngle);
    delay(servoDelayBox);
  }
  else{
    box.write(box.read()-stp);
  }
  delay(servoDelayBox);
}

//MOVE ARM
void armFront(byte stp){
  if (arm.read()-stp<armFrontAngle){
    arm.write(armFrontAngle);
  }
  else{
    arm.write(arm.read()-stp);
  }
  delay(servoDelayArm);
}

void armBack(byte stp){
  if (arm.read()+stp>armBackAngle){
    arm.write(armBackAngle);
  }
  else{
    arm.write(arm.read()+stp);
  }
  delay(servoDelayArm);
}

//SENSORS
bool U(){
  return(!D());
}
bool D(){
  if(digitalRead(switchPin)==HIGH){
    return true;
  }
  return false;
}
bool AB(){
  if(arm.read()==armBackAngle){
    return true;
  }
  return false;
}
bool AT0(){
  if(abs(arm.read()-armTouch0)<=armSpeed){
    return true;
  }
  return false;
}
bool AT1(){
  if(abs(arm.read()-armTouch1)<armSpeed){
    return true;
  }
  return false;
}
bool BO(){
  if(box.read()==boxOpenAngle){
    return true;
  }
  return false;
}
bool BC(){
  if(box.read()==boxCloseAngle){
    return true;
  }
  return false;
}
bool BP(){
  if(abs(box.read()-boxPeekAngle)<2){
    return true;
  }
  return false;
}
bool T0(){
  if (time0){
    return true;
  }
  return false;
}
bool T1(){
  if (time1){
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------------------------------- update servos and counters
void moveAll(){
  if(boxOpening==true){
    boxOpen(boxSpeed);
  }
  if(boxClosing==true){
    boxClose(boxSpeed);
  }
  if (armMovingFoward==true){
    armFront(armSpeed);
  }
  if (armMovingBackwards==true){
    armBack(armSpeed);
  }
  updateTimeCounters();
}
void updateTimeCounters(){
  delay(2);
  counter++; 
  if (counting){
    counter2++;
  }
  
  if (counter%300==0){
    counter=0;
    if (angry>0){
      angry=angry-1;//decrease angry level
    }
  }

  if (counting){   
      if (counter2%timer0==0){        
          time0=true;
        }
        if (counter2%timer1==0){
          time1=true;
        }
    }
}
