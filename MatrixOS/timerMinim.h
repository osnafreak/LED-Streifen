#pragma once

class timerMinim
{
public:
	// объявление таймера с указанием интервала
	timerMinim(uint32_t interval) : _interval(interval), _timer(millis()) {}

	// установка интервала работы таймера
	void setInterval(uint32_t interval) { _interval = interval; }

	// возвращает true, когда пришло время. Сбрасывается в false сам (AUTO) или вручную (MANUAL)
	bool isReady() {
		if (millis() - _timer >= _interval) {
			_timer = millis();
			return true;
		}
		return false;
	}

	// ручной сброс таймера на установленный интервал
	void reset() { _timer = millis(); }							

private:
	uint32_t _timer = 0;
	uint32_t _interval = 0;
};
