# AutBot - Autonomous Robot for Sensory Overload Therapy

---

## Overview

AutBot is a sophisticated autonomous robot designed specifically for sensory overload therapy for children with autism. When a child experiences sensory overload, the robot autonomously approaches them, provides calming sensory feedback (LED light patterns or soft humming sounds), then retreats and repeats the cycle. This gentle, repetitive interaction helps gradually calm the child's sensory system.

---

## How It Works

### Core Therapy Behavior

1. **Detection Phase**: Robot searches for the child using distance sensors
2. **Approach Phase**: When detected, robot gently approaches the child
3. **Therapy Phase**: Robot provides calming sensory input (LED patterns/humming)
4. **Retreat Phase**: Robot backs away to give the child space
5. **Repeat Cycle**: Robot searches again and repeats the calming interaction

This cycle continues until the child's sensory overload subsides, providing a non-threatening, predictable, and soothing presence.

---

## System Architecture

### Hardware Platform

- **Microcontroller**: TI MSP430G2553 (16MHz)
- **Targets**: 
  - LAUNCHPAD (development/testing)
  - NSUMO (production robot platform)
- **Power**: Battery-powered for mobility

### Sensor Systems

#### Distance Sensing (VL53L0X Time-of-Flight Sensors)
- **Purpose**: Detect child's presence and position
- **Configuration**: 5 sensors positioned around robot
  - Front sensor: Primary detection
  - Front-left/Front-right: Precise positioning
  - Left/Right: Side detection (currently disabled due to mounting)
- **Range**: Up to 600mm detection distance
- **Output**: Position classification (front, left, right) and distance ranges (close, mid, far)

#### Line Detection (QRE1113 Analog Sensors)
- **Purpose**: Detect room boundaries and obstacles
- **Configuration**: 4 sensors (front-left, front-right, back-left, back-right)
- **Function**: Prevents robot from hitting walls or leaving designated area
- **Sensitivity**: Detects white lines on dark surfaces

#### IR Remote Control
- **Purpose**: Manual override and therapy session control
- **Protocol**: NEC infrared protocol
- **Commands**: Directional control, start/stop therapy sessions
- **Safety**: Allows caregivers to take manual control when needed

### Drive System

#### Motor Control (TB6612FNG Dual H-Bridge)
- **Motors**: Two independent drive motors for differential steering
- **Control**: PWM-based speed control (0-100% duty cycle)
- **Modes**:
  - Forward/Reverse: Direct movement
  - Rotate Left/Right: In-place turning
  - Arc turns: Smooth curved movements (sharp, mid, wide)

#### Speed Profiles
- **SLOW** (25%): Gentle, non-threatening approach
- **MEDIUM** (45%): Normal operation
- **FAST** (55%): Quick repositioning
- **MAX** (100%): Emergency maneuvers only

---

## Software Architecture

### State Machine Design

The robot operates using a sophisticated finite state machine with 5 core states:

#### 1. WAIT State
- **Purpose**: Idle state waiting for therapy session to begin
- **Triggers**: IR remote command to start
- **Behavior**: Robot remains stationary, LED indicators show ready status

#### 2. SEARCH State
- **Purpose**: Actively locate the child
- **Behavior**: 
  - Alternates between rotation and forward movement
  - Uses input history to remember child's last known position
  - Intelligent search patterns to maximize detection probability
- **Transitions**: 
  - → APPROACH: When child detected
  - → RETREAT: When boundary/obstacle detected
  - → MANUAL: On remote command

#### 3. APPROACH State
- **Purpose**: Gently move toward the child
- **Behavior**:
  - Determines optimal approach path based on child's position
  - Adjusts speed based on distance (slower when closer)
  - Continuous position tracking and path adjustment
- **Sub-states**:
  - Forward: Direct approach
  - Left/Right: Curved approach for better positioning
- **Transitions**:
  - → RETREAT: When boundary detected
  - → SEARCH: When child lost from view

#### 4. RETREAT State
- **Purpose**: Safely move away from boundaries or obstacles
- **Complexity**: Multi-step sequences based on boundary position
- **Strategies**:
  - Reverse: Simple backing away
  - Arc turns: Smooth curved retreats
  - Alignment: Complex multi-move sequences for optimal positioning
- **Safety**: Ensures robot never gets trapped or damaged

#### 5. MANUAL State
- **Purpose**: Caregiver control override
- **Function**: Direct remote control for emergency situations
- **Safety**: Allows immediate intervention when needed

### Core Software Components

#### Input Processing System

```c
struct input {
    struct object object;  // Distance sensor data
    line_e line;          // Boundary detection
};
```

#### State Machine Engine
- **Event-driven**: Processes sensor inputs and generates state transitions
- **Non-blocking**: All operations are non-blocking for responsive behavior
- **Timer-based**: Uses precise timing for movement sequences
- **History-aware**: Maintains input history for intelligent decision making

#### Sensor Fusion
- **Distance Processing**: Converts raw sensor readings to position classifications
- **Line Detection**: Analyzes multiple sensors to determine boundary position
- **Input History**: Ring buffer stores recent sensor readings for pattern recognition

### Key Algorithms

#### Intelligent Search Algorithm

```c
// Uses input history to remember child's last known position
const struct object last_object = 
    input_history_last_directed_object(data->common->input_history);
if (object_at_right(&last_object)) {
    drive_set(DRIVE_DIR_ROTATE_RIGHT, DRIVE_SPEED_FAST);
} else {
    drive_set(DRIVE_DIR_ROTATE_LEFT, DRIVE_SPEED_FAST);
}
```

#### Adaptive Approach Strategy

```c
static approach_state_e next_approach_state(const struct object *object)
{
    if (object_at_front(object)) {
        return APPROACH_STATE_FORWARD;
    } else if (object_at_left(object)) {
        return APPROACH_STATE_LEFT;
    } else if (object_at_right(object)) {
        return APPROACH_STATE_RIGHT;
    }
}
```

#### Complex Retreat Sequences

The retreat system uses predefined move sequences based on boundary position:
- **Front line**: Reverse or alignment maneuvers
- **Back line**: Forward movement
- **Side lines**: Arc turns away from boundary
- **Diagonal lines**: Complex multi-step alignment sequences

---

## Therapy Features

### Sensory Calming Mechanisms

1. **LED Light Therapy**: Gentle, rhythmic light patterns
2. **Auditory Feedback**: Soft humming or gentle sounds
3. **Movement Patterns**: Predictable, smooth robot movements
4. **Proximity Control**: Maintains optimal therapeutic distance

### Safety Features

- **Boundary Detection**: Prevents robot from leaving designated area
- **Collision Avoidance**: Stops motors immediately on error conditions
- **Manual Override**: Caregiver can take control at any time
- **Error Recovery**: Comprehensive error handling and recovery systems

### Therapeutic Benefits

- **Predictable Interaction**: Consistent, repetitive behavior reduces anxiety
- **Non-threatening Presence**: Gentle movements and soft interactions
- **Sensory Regulation**: Provides controlled sensory input to help calm overload
- **Social Distance**: Maintains appropriate physical distance while providing comfort

---

## Development and Testing

### Build System

```bash
# Build for development board
make HW=LAUNCHPAD

# Build for production robot
make HW=NSUMO

# Run specific tests
make HW=LAUNCHPAD TEST=test_drive
```

### Testing Framework

- **Component Tests**: Individual hardware component validation
- **Integration Tests**: Full system behavior testing
- **Hardware Tests**: Platform-specific functionality verification
- **Behavioral Tests**: State machine and interaction pattern validation

### Code Quality

- **Static Analysis**: Automated code quality checks
- **Formatting**: Consistent code style enforcement
- **Documentation**: Comprehensive inline documentation
- **Error Handling**: Robust error detection and recovery

---

## Hardware Requirements

### Essential Components

- MSP430G2553 microcontroller
- 5x VL53L0X distance sensors
- 4x QRE1113 line sensors
- TB6612FNG motor driver
- 2x DC motors with wheels
- IR receiver module
- LED indicators
- Battery pack

### Optional Components

- Speaker/buzzer for auditory feedback
- LED panel for light therapy
- Additional sensors for enhanced detection

---

## Usage Instructions

### For Caregivers

1. **Setup**: Place robot in designated therapy area
2. **Activation**: Use IR remote to start therapy session
3. **Monitoring**: Observe child-robot interaction
4. **Intervention**: Use manual mode if needed
5. **Session End**: Use remote to stop therapy

### For Developers

1. **Environment Setup**: Install MSP430 toolchain
2. **Code Compilation**: Use provided Makefile
3. **Testing**: Run comprehensive test suite
4. **Deployment**: Flash to target hardware
5. **Calibration**: Adjust sensor thresholds as needed

---

## Safety Considerations

### Physical Safety

- Robot movements are gentle and non-threatening
- Emergency stop capability via remote control
- Boundary detection prevents dangerous situations
- Motor shutdown on error conditions

### Therapeutic Safety

- Maintains appropriate social distance
- Provides predictable, non-startling interactions
- Allows child to control interaction through movement
- Supports caregiver oversight and intervention

---

## Future Enhancements

### Planned Features

- **Adaptive Behavior**: Machine learning for personalized therapy
- **Multi-modal Sensing**: Enhanced child detection and interaction
- **Remote Monitoring**: Caregiver notification and session tracking
- **Customizable Therapy**: Adjustable interaction patterns and timing

### Research Applications

- **Sensory Processing Studies**: Data collection on therapy effectiveness
- **Behavioral Analysis**: Interaction pattern analysis
- **Therapeutic Protocol Development**: Evidence-based therapy optimization

---

## Contributing

This project follows strict coding guidelines for reliability and maintainability:

- Consistent naming conventions
- Comprehensive error checking
- Modular design principles
- Extensive testing requirements

---

## License

This project is licensed under the terms specified in the LICENSE file.

---

## Support

For technical support, therapy guidance, or development questions, please refer to the project documentation or contact the development team.

---

*AutBot represents a novel approach to sensory overload therapy, combining robotics, psychology, and engineering to provide compassionate care for children with autism.*