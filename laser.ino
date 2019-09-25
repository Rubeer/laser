

#include <Servo.h>
#include <rgb_lcd.h>

#include "task_scheduler.h"
static task_scheduler<16> Tasks;

#define LDR_POWER_PIN 7
#define LDR_PIN A0
#define SERVO_PIN 5
#define BUZZER_PIN 4

#define LASER_PERIOD 600 
#define BEEP_DURATION 10
#define SERVO_MOVEMENT_DURATION 300
#define SERVO_ANGLE_RETRACTED 3
#define SERVO_ANGLE_EXTENDED 27

static rgb_lcd lcd; // I2C
static Servo servo;

static u32 passersby_count;
static u32 last_counted_time;
static bool laser_was_high;

static u32 servo_action_count;
static bool servo_moving;

void setup()
{
    pinMode(LDR_POWER_PIN, OUTPUT);
    digitalWrite(LDR_POWER_PIN, HIGH);
    pinMode(LDR_PIN, INPUT);

    pinMode(BUZZER_PIN, OUTPUT);

    servo.attach(SERVO_PIN);
    servo.write(SERVO_ANGLE_RETRACTED);

    lcd.begin(16,2);
    lcd.setCursor(0,0); lcd.print(0);
    lcd.setCursor(0,1); lcd.print("Passanten");
}

void loop()
{
    Tasks.Update();

    u32 now = millis();
    u16 ldr = analogRead(LDR_PIN);

    bool laser_low  = ldr > 550;
    bool laser_high = ldr < 450;

    if(laser_low)
    {
        lcd.setRGB(0,0,255);
    }

    if(laser_high)
    {
        laser_was_high = true;
        lcd.setRGB(0,255,0);
    }

    bool laser_period_elapsed = (now - last_counted_time) > LASER_PERIOD;

    if(laser_low && laser_was_high && laser_period_elapsed)
    {
        ++passersby_count;
        laser_was_high = false;
        last_counted_time = now;

        lcd.setCursor(0,0);
        lcd.print(passersby_count);

        digitalWrite(BUZZER_PIN, HIGH);
        Tasks.ExecuteAfter(BEEP_DURATION, []
        {
            digitalWrite(BUZZER_PIN, LOW);
        });
    }

    if(!servo_moving && (servo_action_count < passersby_count))
    {
        servo.write(SERVO_ANGLE_EXTENDED);
        servo_moving = true;
        Tasks.ExecuteAfter(SERVO_MOVEMENT_DURATION, []
        {
            servo.write(SERVO_ANGLE_RETRACTED);
            Tasks.ExecuteAfter(SERVO_MOVEMENT_DURATION, []
            {
                ++servo_action_count;
                servo_moving = false;
            });
        });
    }

}
