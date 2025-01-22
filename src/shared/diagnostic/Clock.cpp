#include "Clock.hpp"
#include "clock.hpp"
#include <algorithm>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

std::map<const char*, ClockReport> reportedClocks;

void ClockReport::AddTime(double time)
{
	averageTime = (averageTime + time) / 2.0;
}

double ClockReport::GetAverageTime() { return averageTime; }

void Clock::Stop()
{
	if (released)
		return;

	released = true;
	double timeElapsed = static_cast<double>(SDL_GetPerformanceCounter() - start) / freq;

	Report(timeElapsed);
}

void Clock::Report(double elapsedTime)
{
	if (reportedClocks.contains(name))
	{
		reportedClocks[name].AddTime(elapsedTime);
	}
	else
	{
		reportedClocks[name] = { elapsedTime };
	}
}

void ShowClockReports()
{
	std::cout << "Clock reports: " << std::endl;

	std::vector<std::pair<const char*, ClockReport>> items(reportedClocks.begin(), reportedClocks.end());

	std::sort(items.begin(), items.end(), [] (auto& a, auto& b) {

		return a.second.GetAverageTime() > b.second.GetAverageTime();

	});

	for(auto [name, rep] : items)
	{
		boost::format fmt("\t'%s' \t average performance time: %.8f");

		std::cout << fmt % name % rep.GetAverageTime() << std::endl;
	}

	reportedClocks.clear();
}