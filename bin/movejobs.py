#move output files back to where they started

import os
import shutil

noofjobs = 1023
startval = 0

for jobno in xrange(noofjobs):
   jobno = jobno
   job = "job"+str(jobno)+".root"
   jobdir = "job"+str(jobno)
   destination = "PandoraCondor/work/"+jobdir
   if (os.path.exists(job)):
      shutil.move(job, destination)
      os.chdir(destination)
      os.rename(job, "output.root")
      os.chdir("../../../")

for jobno in xrange(noofjobs):
   jobno = jobno
   job = "job"+str(jobno)+"2.root"
   jobdir = "job"+str(jobno)
   destination = "PandoraCondor/work/"+jobdir
   if (os.path.exists(job)):
      shutil.move(job, destination)
      os.chdir(destination)
      os.rename(job, "output2.root")
      os.chdir("../../../")


for jobno in xrange(noofjobs):
   jobno = jobno
   job = "job"+str(jobno)+"3.root"
   jobdir = "job"+str(jobno)
   destination = "PandoraCondor/work/"+jobdir
   if (os.path.exists(job)):
      shutil.move(job, destination)
      os.chdir(destination)
      os.rename(job, "output3.root")
      os.chdir("../../../")

for jobno in xrange(noofjobs):
   jobno = jobno
   job = "job"+str(jobno)+"4.root"
   jobdir = "job"+str(jobno)
   destination = "PandoraCondor/work/"+jobdir
   if (os.path.exists(job)):
      shutil.move(job, destination)
      os.chdir(destination)
      os.rename(job, "output4.root")
      os.chdir("../../../")


for jobno in xrange(noofjobs):
   jobno = jobno
   job = "job"+str(jobno)+"5.root"
   jobdir = "job"+str(jobno)
   destination = "PandoraCondor/work/"+jobdir
   if (os.path.exists(job)):
      shutil.move(job, destination)
      os.chdir(destination)
      os.rename(job, "output5.root")
      os.chdir("../../../")



for jobno in xrange(noofjobs):
   jobno = jobno
   job = "job"+str(jobno)+"d.root"
   jobdir = "job"+str(jobno)
   destination = "PandoraCondor/work/"+jobdir
   if (os.path.exists(job)):
      shutil.move(job, destination)
      os.chdir(destination)
      os.rename(job, "outputd.root")
      os.chdir("../../../")
    
