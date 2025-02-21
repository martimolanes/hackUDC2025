#ifndef VIBRATOR_HW_H
#define VIBRATOR_HW_H

#ifdef __cplusplus
extern "C" {
#endif

void vibrator_init(void);
void vibrator_control(const char *cmd);

#ifdef __cplusplus
}
#endif

#endif /* VIBRATOR_HW_H */
