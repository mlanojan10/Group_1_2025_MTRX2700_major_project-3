# Group_1_2025_MTRX2700_major_project-3

## Details about the project
### Group Member Roles & Responsibilities:

- **Sharon Britto**  
  Responsible for Riddle & Intergation Module. Wrote code and user instructions.

- **Lillie Mellin**  
  Responsible for TSC Module. Wrote code and user instructions.

- **Luc Do**  
 Responsible for Potentiometer & User Interface . Wrote code and user instructions.

- **Melvin Lanojan**  
 Responsible for Lidar Module. Wrote code and user instructions.

- **Jason Yang**  
 Responsible for Timer Module. Wrote code and user instructions.


![Image](https://github.com/user-attachments/assets/46e8d0d3-676f-463e-9957-3b139e92b115)
Hi Adventurer! Welcome to Pirates of the C-ribbean! 
This user guide will take you through the instructions of the game, and then we will dive deeper into what makes up its functionality. 

## Instructions of the Game 
You have set sail to find the hidden treasure - but there are 4 challenges you must first complete before you get the gold! 

### Navigating Roacky Shores 
Getting to the treasure is not easy, and your first obstacle is steering clear of any obstacles and getting to the next island…

### Riddle Island 
Woohoo! You have managed to get to the first island. To unlock a clue to the next island, you have to correctly answer a few mind boggling questions. 

You will be presented with one riddle that you need to answer correctly. Input your answer into the text box after reading the question. You do not need to worry about capatalising, just ensure your answer is spelt correctly. You will be able to request for a hint after a certain amount of time, but to do this, you need to locate the hidden button in the island. 

Once you have completed this, you will be asked to answer a quick maths questions. You will then need to apply a Ceaser Cipher shift of this value to the last riddle answer you had. Completing all these challenges unlocks a further island, and a step closer to the treasure!

### Blinking Light Shores 

### Treasure Island


## Code Description
### Lidar Module 
#### Testing

### Riddle Module 
![Image](https://github.com/user-attachments/assets/328baa93-9c68-4ef6-bf3b-23cdceb6b2ea)

This module constructs an interactive riddle game utilsizing UART communication to interact with the user via a serial terminal. The gameplay consists of three stages: answering a riddle, solving a basic math problem, and applying a Caesar cipher. The riddle is selected randomly from a predefined array of Riddle structs, wcih contans the riddle,its answer and two hints, and the player must respond correctly to proceed to the next stage.The game flow is managed within the riddle_game() function, which contains the main loop that processes user input and tracks progression through each challenge step. The functionality is controlled by if-else statements and while loops as its basis. 

Key supporting functions include AskNewRiddle(), which selects and displays a random riddle; AskMathQuestion(), which generates a simple addition problem; and AskCaesarChallenge(), which prompts the user to encode the riddle’s answer using a Caesar cipher. The Caesar cipher logic itself is implemented in CaesarCipher(), which shifts each character in the input string by a specified amount, defined by the generated math answer. Controls are placed within both AskNewRiddle() and AskMathQuestion() function that ensures no two riddles are the same and the sum is not 0:

```
static void AskNewRiddle(void) {
  …

    do {
        index = rand() % NUM_RIDDLES;
    } while (index == last_index);  // Ensure it’s not the same as the last one
…
}

static void AskMathQuestion(void) {
    do {
        math_1 = rand() % 5;
        math_2 = rand() % 5;
        math_answer = math_1 + math_2;
    } while (math_answer == 0);  // Avoid 0 + 0

 …
}

```
To ensure efficient input comparison, all strings received by UART are converted to lowercase before processing via ToLowerCase() function. This allows users to enter answers without worrying about letter casing, simplifying the input format and reducing the chance of errors due to grammar. All user input is captured from the UART interface using SerialGetChar() and echoed back with SerialOutputChar(). Once the module is completed, the module updates the global variable game_progress, which is a key component to the progression of the game. 

For additional complexity of the module, the use of hints is included, providing the user with support if needed. This is done through the use of a physical button input that provides a readable high signal that is taken as an input and compared within the code. This input is read by input in PE7, after configuring the pin and adding a pull down resistor for accuracy. The hint is only provided after a certain time limit, which is controlled by an internal clock SysTick_Init() and a rising edge of the input pin. If not already found, the user is notified, and hints are able to be requested at 10 second intervals. 
```
if (!button_hint_shown && elapsed >= 1000 && riddle_step == 0 && !hint_button_used) {
           printf("\r\nThere's a button hidden on this island. Find it and gain a hint!\r\n\n> ");
           button_hint_shown = 1;
       }
       // === Hint button check (PE7) ===
       if (riddle_step == 0 && (GPIOE->IDR & GPIO_IDR_7) != 0) {
           hint_button_used = 1;
	    // Conditions to be met to provide a hint


          // Code for providing a hint
       }
```
If the user answers the riddle correctly within a short time period, the AskNewRiddle() function is called once more and provides the user with a new Riddle and any corresponding hints. 

#### Testing
To test the functionality of the module, certain tests were conducted and compared to the known expected behaviour. If these are aligned correctly, the module can be deemed as functional. Please note, the inputs answer and caesar are dependent on the riddle, and is only stated to demonstrate testing outcomes: 

<table>
  <thead>
    <tr>
      <th>Sub module</th>
      <th>User Input</th>
      <th>Expected Output</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td rowspan="8">Riddle</td>
      <td>answer &gt; 45 seconds</td>
      <td>Correct! On to the next challenge…<br>Now answer this: What is 4 + 2?</td>
    </tr>
    <tr>
      <td>ANswER</td>
      <td>Correct! On to the next challenge…<br>Now answer this: What is 4 + 2?</td>
    </tr>
    <tr>
      <td>answer &lt; 45 secs</td>
      <td>Correct! Wow, you're quick. So here's another one!</td>
    </tr>
    <tr>
      <td>Wanswer</td>
      <td>Wrong! Try again:</td>
    </tr>
    <tr>
      <td>answer1</td>
      <td>Wrong! Try again:</td>
    </tr>
    <tr>
      <td>PE7 high &gt; 45 seconds</td>
      <td>Here’s a hint to help you: Hint</td>
    </tr>
    <tr>
      <td>PE7 high &lt; 45 seconds</td>
      <td>You have to wait 45 seconds before getting a hint.</td>
    </tr>
    <tr>
      <td>PE7 low &gt; 45 seconds</td>
      <td>There's a button hidden on this island. Find it and gain a hint!</td>
    </tr>
    <tr>
      <td rowspan="4">Math Question</td>
      <td>6</td>
      <td>Nicely done! Final challenge…<br>Final task! Enter the Caesar cipher of the riddle answer with a shift of 6.</td>
    </tr>
    <tr>
      <td>5</td>
      <td>Incorrect. What is 4 + 2?</td>
    </tr>
    <tr>
      <td>A</td>
      <td>Incorrect. What is 4 + 2?</td>
    </tr>
    <tr>
      <td>HellO</td>
      <td>Incorrect. What is 4 + 2?</td>
    </tr>
    <tr>
      <td rowspan="3">Caesar Cipher</td>
      <td>ceaser</td>
      <td>Well done! You've completed the riddle island!</td>
    </tr>
    <tr>
      <td>CeASER</td>
      <td>Well done! You've completed the riddle island!</td>
    </tr>
    <tr>
      <td>wceaser</td>
      <td>Not quite! Try the Caesar cipher again with a shift of %6</td>
    </tr>
  </tbody>
</table>

### TSC Module 
#### Testing


### Potentiometer Module 
#### Testing



### Timer Module 
#### Testing


### Integration 

![Image](https://github.com/user-attachments/assets/4a3fe5c0-dc13-4e34-8cef-2b5e24a87752)

The integration combines all individual modules to compose a fully functional game. The main components that control this is the current island and the game progress. To control these, the current island that the user is positioned on comes from an LDR-Op Amp signal that acts as a Analog to Digital converter, andis configured to provide a high signal when the island LDR is covered. Additionally, pins on the STM are configured as inputs and are connected to the op amp, to connect the code to the physical circuit. The below states the exact pins and the island/module it is connected to. Additionally, the game_progress variable is a volatile uint8_t bitfield that tracks which "islands" have been completed and is globally accessible to all modules. 

<table>
  <thead>
    <tr>
      <th>Pin</th>
      <th>Associated Module</th>
      <th>Game Progress Variable</th>
      <th>Condition to Activate</th>
      <th>After Completion</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>PE11</td>
      <td>Lidar Module</td>
      <td>0b0000</td>
      <td>0b0000</td>
      <td>0b0001</td>
    </tr>
    <tr>
      <td>PA2</td>
      <td>Riddle Module</td>
      <td>0b0001</td>
      <td>0b0001</td>
      <td>0b0010</td>
    </tr>
    <tr>
      <td>PA3</td>
      <td>TSC Module</td>
      <td>0b0010</td>
      <td>0b0010</td>
      <td>0b0100</td>
    </tr>
    <tr>
      <td>PE9</td>
      <td>Potentiometer Module</td>
      <td>0b0100</td>
      <td>0b0100</td>
      <td>0b1000</td>
    </tr>
  </tbody>
</table>

The main function then monitors state changes ("rising edges") on 4 GPIO pins, in which attempts to launch a minigame only if the previous one has been completed. This means the game_progress variable and the condition to activate the module is compared within an if statement for every module, seen within the example logic:

```
if (pa2 && !prev_pa2) {
    if (game_progress == 0b0001) {
        riddle_game();
        game_progress = 0b0010;
    } else {
        printf("\r\nYou must complete the previous lidar island first!\r\n");
    }
}
```
If the statement is not true, a message will be sent to the user and the system will continue looking for the next input. To ensure accurate functionality, each input has a prev_* variable that stores the previous state of that input from the last loop iteration, and is compared to the current state to detect a new press. 
```
if (pe11 && !prev_pe11) {
    // Rising edge detected — Lidar Module 
    lidar_game()
}
```
The combination of the individual modules and these process handlers enable a fully integrated treasure hunt game, which interacts successfully with the user. It prevents any games to be skipped, and the story to follow a linear controlled path, as well as outputs responses in a clean neat manner. 

#### Testing
To test the functionality of the system, the expected outcomes were compared to those observed, and altered accordingly: 
<table>
  <thead>
    <tr>
      <th>Current <code>game_progress</code></th>
      <th>Input Triggered</th>
      <th>Expected Output</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td rowspan="2">0b0000</td>
      <td>PE11</td>
      <td><code>lidar_game()</code> runs</td>
    </tr>
    <tr>
      <td>PA2, PA3, PE9</td>
      <td>You cannot do this island!</td>
    </tr>
    <tr>
      <td rowspan="2">0b0001</td>
      <td>PE11</td>
      <td>You cannot do this island!</td>
    </tr>
    <tr>
      <td>PA2</td>
      <td><code>riddle_game()</code> runs</td>
    </tr>
    <tr>
      <td>0b0001</td>
      <td>PE11, PA3, PE9</td>
      <td>You cannot do this island!</td>
    </tr>
    <tr>
      <td rowspan="2">0b0010</td>
      <td>PA3</td>
      <td><code>TSC_game()</code> runs</td>
    </tr>
    <tr>
      <td>PE11, PA2, PE9</td>
      <td>You cannot do this island!</td>
    </tr>
    <tr>
      <td rowspan="2">0b0100</td>
      <td>PE9</td>
      <td><code>potentiometer_game()</code> runs</td>
    </tr>
    <tr>
      <td>PE11, PA32, PA3</td>
      <td>You cannot do this island!</td>
    </tr>
    <tr>
      <td>0b1000</td>
      <td>PE11, PA32, PA3, PE9</td>
      <td>No response</td>
    </tr>
    <tr>
      <td>Any</td>
      <td>PE11 held for 2 seconds</td>
      <td>Only triggers response once (rising edge)</td>
    </tr>
    <tr>
      <td>Any</td>
      <td>Rapidly tapping PE11</td>
      <td>Only triggers response after every second</td>
    </tr>
  </tbody>
</table>
