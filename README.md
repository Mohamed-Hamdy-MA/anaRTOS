# anaRTOS: A Custom Real-Time Operating System for ARM Cortex-M

## Introduction
anaRTOS is a lightweight, real-time operating system (RTOS) designed for ARM Cortex-M microcontrollers. It provides task scheduling, inter-task communication, and synchronization mechanisms. The project focuses on efficient **task managemen**t with support for **priority-based preemption**, **round-robin scheduling**, and **priority inheritance semaphores**.

This RTOS is modular, scalable, and suitable for applications requiring precise timing and high reliability.

---

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Dependencies](#dependencies)
- [APIs](#apis)
- [Troubleshooting](#troubleshooting)
- [Contributors](#contributors)
- [License](#license)

---

## Features

- Priority-based preemptive task scheduling
- Round-robin scheduling for tasks with equal priority
- Task creation, activation, termination, and holding
- Semaphore-based synchronization
- Idle task for system background operations
- Exception handling for system faults
- Support for Privileged/Unprivileged modes

---

## Installation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Mohamed-Hamdy-MA/anaRTOS.git
   ```

2. **Include the following file in your project:**
   - `RTOS_Scheduler.h`

3. **Ensure your development environment supports ARM Cortex-M**

---

## Usage

1. **Initialize the RTOS:**
   ```c
    anaRTOS_Init();
   ```

2. **Create a Task:** Define the task configuration and create a new task:
   ```c
	strcpy(Task1.TaskUserConfig.Task_Name, "Task 1");
	Task1.TaskUserConfig.Task_InputPriority = 0;	// High priority
	Task1.TaskUserConfig.Task_ProgramEntry = &Task1Func;
	Task1.TaskUserConfig.Task_StackSize = 100;	// 100 Bytes
	anaRTOS_CreateTask(&Task1);
   ```
3. **RTOS hardware initialization:**
   ```c
   RTOS_HW_init();
4. **Start the RTOS:**
   ```c
   anaRTOS_StartOS();
   ```

4. **Inter-task Communication:** Use semaphores for synchronization:
   ```c
   anaRTOS_AcquireSemaphore(&mySemaphoreConfig);
   anaRTOS_ReleaseSemaphore(&mySemaphoreConfig);
   ```

---

## Configuration
### Memory

Modify the MSP stack size in `RTOS_CMxPorting.h`:
```c
#define anaRTOS_MSPStackSize 2048U
```

---

## Dependencies

- **CMSIS Core:** Required for ARM Cortex-M functionalities.
- **Utils folder:** Required for anaRTOS Queues functionalities.
- **Standard Libraries:** `<stdio.h>`, `<stdint.h>`, and `<string.h>`.

---

## APIs

### Initialization
- `void anaRTOS_Init(void);`
- `void anaRTOS_StartOS(void);`

### Task Management
- `void anaRTOS_CreateTask(RTOS_TaskConfig_t* TaskConfig);`
- `void anaRTOS_ActivateTask(RTOS_TaskConfig_t* TaskConfig);`
- `void anaRTOS_TerminateTask(RTOS_TaskConfig_t* TaskConfig);`
- `void anaRTOS_HoldTask(RTOS_TaskConfig_t* TaskConfig, uint32_t NoOfTicks);`

### Semaphore Management
- `void anaRTOS_AcquireSemaphore(RTOS_SemaphoreConfig_t* SemaphoreConfig);`
- `void anaRTOS_ReleaseSemaphore(RTOS_SemaphoreConfig_t* SemaphoreConfig);`

### Exception Handlers
- `void HardFault_Handler(void);`
- `void MemManage_Handler(void);`
- `void BusFault_Handler(void);`
- `void UsageFault_Handler(void);`

---

## Troubleshooting

### Common Issues

- **Hard Fault:**
  - Ensure all task stacks are correctly initialized.
  - Verify task priorities do not conflict.

- **Semaphore Deadlock:**
  - Check semaphore priorities and ownership.

### Debugging

Enable system logging and use an appropriate debugger to trace execution.

---

## Contributors

- **Mohamed Hamdy** - Auther and Developer.

---

## License

This project is licensed under the MIT License. See the [LICENSE](./LICENSE) file for details.
