

// Robin Vos 24-09-19

#include <stdint.h>
typedef uint32_t u32;
typedef uint8_t u8;

#define TaskCallback(name) void name(void)
typedef TaskCallback(task_callback);


template<int MAX_TASK_COUNT>
struct task_scheduler
{

    struct task
    {
        // NOTE(robin): Could save some space and store the end time (StartTime+Interval),
        // but that might require some extra logic to handle millis() wraparound
        u32 StartTime;
        u16 Interval;

        task_callback *Callback;
    };

    task Tasks[MAX_TASK_COUNT];
    u8 TaskCount;
    u32 CurrentTime;

    inline bool ExecuteIfElapsed(task *Task)
    {
        if((CurrentTime - Task->StartTime) > Task->Interval)
        {
            Task->Callback();
            return true;
        }
        return false;
    }

    inline void Update()
    {
        CurrentTime = millis();

        for(u8 i = 0; i < TaskCount; ++i)
        {
            if(ExecuteIfElapsed(Tasks + i))
            {
                // NOTE(robin): unordered removal (replace with last item)
                if(i != --TaskCount)
                {
                    Tasks[i--] = Tasks[TaskCount];
                }
            }
        }
    }

    inline void ExecuteAfter(u16 Interval, task_callback *Callback)
    {
        task Task = {CurrentTime, Interval, Callback};

        for(;;)
        {
            if(TaskCount < MAX_TASK_COUNT)
            {
                Tasks[TaskCount++] = Task;
                return;
            }

            if(ExecuteIfElapsed(&Task))
            {
                return;
            }
            else
            {
                Update();
            }
        }
    }
};
