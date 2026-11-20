
#include "NRL.h"

// ---------------------------------------------------------------------------
// ARM ANGLES
// ---------------------------------------------------------------------------
static const float ARM_ORIGINAL = 95.0f;
static const float ARM_FINAL = 35.0f;

// ---------------------------------------------------------------------------
// GRIPPER ANGLES
// ---------------------------------------------------------------------------
static const float GRIPPER_ORIGINAL = 35.0f;
static const float GRIPPER_FINAL = 120.0f;

// ---------------------------------------------------------------------------
// DRIVE MOTORS
// ---------------------------------------------------------------------------
static HexaDCMotor leftMotor{{.dirPin = MOTOR_L_DIR,
                              .pwmPin = MOTOR_L_PWM,
                              .flipped = true}};

static HexaDCMotor rightMotor{{.dirPin = MOTOR_R_DIR,
                               .pwmPin = MOTOR_R_PWM,
                               .flipped = true}};

// ---------------------------------------------------------------------------
// SERVOS
// GRIPPER = SERVO_1
// ARM = SERVO_2
// ---------------------------------------------------------------------------
static HexaServo gripper{{.signalPin = SERVO_1,
                          .startAngle = GRIPPER_ORIGINAL,
                          .settleMs = 0}};

static HexaServo arm{{.signalPin = SERVO_2,
                      .startAngle = ARM_ORIGINAL,
                      .settleMs = 0}};

static HexaServo servo3{{.signalPin = SERVO_3,
                         .startAngle = 0.0f,
                         .settleMs = 0}};

static HexaServo servo4{{.signalPin = SERVO_4,
                         .startAngle = 180.0f,
                         .settleMs = 0}};

// ---------------------------------------------------------------------------
// SETTINGS
// ---------------------------------------------------------------------------
static const float SLOW_SCALE = 0.5f;
static const float LOW_BATTERY_V = 10.5f;
static const unsigned long SERVO_DELAY = 400;

class TankTeleop : public NRLOpMode
{
    // 0 = Gripper FINAL
    // 1 = Arm FINAL
    // 2 = Gripper ORIGINAL
    // 3 = Arm ORIGINAL
    int _servoStep = 0;

    bool _waitingForDelay = false;
    unsigned long _servoMoveTime = 0;

public:
    void init() override
    {
        leftMotor.begin();
        rightMotor.begin();

        gripper.begin();
        arm.begin();

        servo3.begin();
        servo4.begin();
    }

    void loop() override
    {
        // ---- TANK DRIVE -----------------------------------------------

        float scale = gamepad1.pressed(BTN_RB)
                          ? SLOW_SCALE
                          : 1.0f;

        int leftSpeed =
            (int)(gamepad1.leftY() * 255.0f * scale);

        int rightSpeed =
            (int)(gamepad1.rightY() * 255.0f * scale);

        leftMotor.setSpeed(leftSpeed);
        rightMotor.setSpeed(rightSpeed);

        // ---- X: RESET ARM AND GRIPPER -------------------------------

        if (gamepad1.justPressed(BTN_X))
        {
            // Return both servos to original positions
            gripper.setPosition(GRIPPER_ORIGINAL);
            arm.setPosition(ARM_ORIGINAL);

            // Reset LB sequence
            _servoStep = 0;

            // Apply 400 ms delay
            _waitingForDelay = true;
            _servoMoveTime = millis();
        }

        // ---- LB: FOUR-STEP GRIPPER / ARM SEQUENCE ---------------------

        // Wait 400 ms after each movement
        if (_waitingForDelay &&
            millis() - _servoMoveTime >= SERVO_DELAY)
        {
            _waitingForDelay = false;
        }

        // Accept next LB press only after delay
        if (gamepad1.justPressed(BTN_LB) && !_waitingForDelay)
        {
            switch (_servoStep)
            {
            case 0:
                // First press: Gripper FINAL
                gripper.setPosition(GRIPPER_FINAL);
                break;

            case 1:
                // Second press: Arm FINAL
                arm.setPosition(ARM_FINAL);
                break;

            case 2:
                // Third press: Gripper ORIGINAL
                gripper.setPosition(GRIPPER_ORIGINAL);
                break;

            case 3:
                // Fourth press: Arm ORIGINAL
                arm.setPosition(ARM_ORIGINAL);
                break;
            }

            // Start 400 ms delay after movement command
            _servoMoveTime = millis();
            _waitingForDelay = true;

            // Advance to next step
            _servoStep = (_servoStep + 1) % 4;
        }

        // ---- RIGHT TRIGGER: SERVO 3 / SERVO 4 --------------------------

        if (gamepad1.pressed(BTN_RT))
        {
            servo3.setPosition(110.0f);
            servo4.setPosition(70.0f);
        }
        else
        {
            servo3.setPosition(0.0f);
            servo4.setPosition(180.0f);
        }

        // ---- BATTERY LED ----------------------------------------------

        float v = power.getBatteryVoltage();

        if (v < LOW_BATTERY_V)
            userLed.setBlink(255, 0, 0, 250);
        else
            userLed.setSolid(0, 255, 0);

        // ---- TELEMETRY -----------------------------------------------

        telemetry.addData("left", (float)leftSpeed);
        telemetry.addData("right", (float)rightSpeed);
        telemetry.addData("scale", scale);
        telemetry.addData("battery", v);
        telemetry.addData("servo step", (float)_servoStep);
    }

    void stop() override
    {
        leftMotor.setSpeed(0);
        rightMotor.setSpeed(0);

        userLed.setOff();
    }
};

REGISTER_OPMODE(TankTeleop, "Zenox_v3", TELEOP);

// -- made by vivaan
