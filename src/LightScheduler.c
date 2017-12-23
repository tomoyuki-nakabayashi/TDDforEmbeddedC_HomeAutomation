#include <stdbool.h>
#include "LightScheduler.h"
#include "LightController.h"
#include "TimeService.h"

enum
{
  UNUSED = -1,
  TURN_OFF,
  TURN_ON
};

typedef struct {
  int id;
  int minuteOfDay;
  int event;
  Day day;
} ScheduledLightEvent;

static ScheduledLightEvent scheduledEvent;

static void scheduleEvent(int id, Day day, int minuteOfDay, int event)
{
  scheduledEvent.id = id;
  scheduledEvent.event = event;
  scheduledEvent.minuteOfDay = minuteOfDay;
  scheduledEvent.day = day;
}

void LightScheduler_ScheduleTurnOn(int id, Day day, int minuteOfDay)
{
  scheduleEvent(id, day, minuteOfDay, TURN_ON);
}

void LightScheduler_ScheduleTurnOff(int id, Day day, int minuteOfDay)
{
  scheduleEvent(id, day, minuteOfDay, TURN_OFF);
}

static void operateLight(ScheduledLightEvent *lightEvent)
{
  if(lightEvent->event == TURN_ON)
    LightController_On(lightEvent->id);
  else if(lightEvent->event == TURN_OFF)
    LightController_Off(lightEvent->id);
}

static bool DoesLightRespondToday(const int today, const int reactionDay)
{
  if(reactionDay == EVERYDAY)
    return true;
  
  if(reactionDay == today)
    return true;

  if((reactionDay == WEEKEND) && (today == SATURDAY || today == SUNDAY))
    return true;
  
  if((reactionDay == WEEKDAY) && (today >= MONDAY && today <= FRIDAY))
    return true;

  return false;
}

static void processEventDueNow(Time *time, ScheduledLightEvent *lightEvent)
{
  int reactionDay = lightEvent->day;
  Day today = time->dayOfWeek;
  if(lightEvent->id == UNUSED)
    return;
  if(DoesLightRespondToday(today, reactionDay) == false)
    return;
  if(lightEvent->minuteOfDay != time->minuteOfDay)
    return;
  
  operateLight(lightEvent);
}

void LightScheduler_Wakeup(void)
{
  Time time;
  TimeService_GetTime(&time);
  processEventDueNow(&time, &scheduledEvent);
}

void LightScheduler_Create(void)
{
  scheduledEvent.id = UNUSED;

  TimeService_SetPeriodicAlarmInSeconds(60, LightScheduler_Wakeup);
}

void LightScheduler_Destory(void)
{
  TimeService_CancelPeriodicAlarmInSeconds(60, LightScheduler_Wakeup);
}