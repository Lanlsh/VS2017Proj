#pragma once
class PipeClient
{
public:
	PipeClient();
	~PipeClient();

	void StartPipeClient();
	void StopPipeClient() { m_bIsStopLoop = true; }

private:
	bool m_bIsStopLoop;
};

