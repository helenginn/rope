// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__SerialJob__
#define __vagabond__SerialJob__

#include "engine/Handler.h"
#include "RopeJob.h"

/** \class SerialJob
 *  will organise multi-threaded refinement on models according to
 *  a particular job type. Caller should be a sublcass of SerialJobResponder
 *  in which case the models associated with the first thread will be 
 *  sent to the caller before refinement and again before unloading,
 *  for monitoring purposes */

class Model;
class Entity;
class ThreadWorksOnModel;

class SerialJobResponder
{
public:
	virtual ~SerialJobResponder() {};
	virtual void attachModel(Model *model) = 0;
	virtual void detachModel(Model *model) = 0;
	virtual void updateModel(Model *model, int idx) = 0;

	virtual void finishedModels() = 0;
};

class SerialJob : public Handler
{
public:
	SerialJob(Entity *entity, SerialJobResponder *responder = nullptr);

	/** set white list of models to refine. If not set/empty vector,
	 * will refine all models associated with entity */
	void setModelList(std::vector<Model *> models)
	{
		_models = models;
	}
	
	/** set type of job to do on models */
	void setRopeJob(const rope::RopeJob job)
	{
		_job = job;
	}
	
	/** set how many threads to run jobs on */
	void setThreads(const int threads)
	{
		_threads = threads;
	}
	
	size_t threadCount() const
	{
		return _threads;
	}
	
	size_t modelCount() const
	{
		return _models.size();
	}
	
	/** prepare all objects etc. ready to start */
	void setup();
	
	/** begin the actual jobs on thread workers */
	void start();
	
	/** call when done, allowing serial job to clean up */
	void waitToFinish();
	
	/** number of models finished */
	size_t finishedCount()
	{
		return _finished;
	}

	friend ThreadWorksOnModel;
protected: // thread works on model
	Model *acquireModel();

	void attachModel(Model *model);
	void detachModel(Model *model);
	void updateModel(Model *model, int idx);
	
	void incrementFinished();
private:
	void settings();
	void prepareThreads();

	SerialJobResponder *_responder = nullptr;
	rope::RopeJob _job = rope::Refine;
	Entity *_entity = nullptr;
	int _threads = 1;

	std::mutex _mutex;

	std::vector<Model *> _models;
	
	int _finished = 0;
	Pool<Model *> _pool;
};

#endif
