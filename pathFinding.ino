void setup() {
  // put your setup code here, to run once:

}

  String data2 = Communications.recieve();
  int prevFloor = currFloor;

  void loop() {
  // put your main code here, to run repeatedly:

  // MVP Implementation
  if(data2.equals("")){
    elevatorMovement("S");
    elevatorStatus = 0
    
  } else if(data2.equals("U") && currFloor != 5){
    data = Communications.recieve();
    elevatorMovement("U");
    elevatorStatus = 1;

    if(prevFloor < currFloor){
      elevatorMovement("S");
      data2 = "";
      elevatorStatus = 0
    }
  } else if(data2.equals("D") && currFloor != 1){
    data = Communications.recieve();
    elevatorMovement("D");
    elevatorStatus = -1;

    if(prevFloor > currFloor){
      elevatorMovement("S");
      data2 = "";
      elevatorStatus = 0
    }
  } else if(data.equals("E"){
      elevatorMovement("E")
  }
  
  //Version 1 implementation
  
  switch(queueCase){
    //Where neither queue has anything in it
    case 0:
      elevatorMovement("S");
      elevatorStatus = 0;
      break;

    //Where queueFloorsBelow is the only queue that is filled
    case 1:
      while(queueFloorsBelow.count() != 0){
        data = Communications.recieve();
        elevatorMovement("D");
        elevatorStatus = -1;
        if(data.equals("E"){
          elevatorMovement("E");
          elevatorStatus = 0;
        }

        if(queueFloorsBelow.peek() == currFloor){
          elevatorMovement("S");
          queueFloorsBelow.pop();
          elevatorStatus = 0;
          delay(3000);
        }  
      }
      queueCase = 0;
      break;

    //Where queueFloorsAbove is the only queue that is filled
    case 2:
      while(queueFloorsAbove.count() != 0){
        data = Communications.recieve();
        elevatorMovement("U");
        elevatorStatus = 1;
        if(data.equals("E"){
          elevatorMovement("E");
          elevatorStatus = 0;
        }

        if(queueFloorsAbove.peek() == currFloor){
          elevatorMovement("S");
          queueFloorsAbove.pop();
          elevatorStatus = 0;
          delay(3000);
        }
      }
    queueCase = 0;
    break;

    //Where queueFloorsAbove was the first queue filled, then queueFloorsBelow
    case 3:
      while(queueFloorsAbove.count() != 0){
        data = Communications.recieve();
        elevatorMovement("U");
        elevatorStatus = 1;
        if(data.equals("E"){
          elevatorMovement("E");
          elevatorStatus = 0;
        }

        if(queueFloorsAbove.peek() == currFloor){
          elevatorMovement("S");
          queueFloorsAbove.pop();
          elevatorStatus = 0;
          delay(3000);
        }
      }
    queueCase = 1;
    break;

    //Where queueFloorsBelow was the first queue filled, then queueFloorsAbove
    case 4:
      while(queueFloorsBelow.count() != 0){
        data = Communications.recieve();
        elevatorMovement("D");
        elevatorStatus = -1;
        if(data.equals("E"){
          elevatorMovement("E");
          elevatorStatus = 0;
        }

        if(queueFloorsAbove.peek() == currFloor){
          elevatorMovement("S");
          queueFloorsAbove.pop();
          elevatorStatus = 0;
          delay(3000);
        }
      }
    queueCase = 2;
    break;
 
    }
  }
}
