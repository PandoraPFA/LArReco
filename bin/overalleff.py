#find the total efficiency

import ROOT
from ROOT import TCanvas, TGraph, TGaxis
from ROOT import TH2F
from ROOT import gROOT
import numpy as np 
import math as m
import os
import shutil
import time


file = ROOT.TFile.Open("tchain.root", "update")

noofjobs = 1023
minus = 0
chain = ROOT.TChain("ttreed");
for jobno in xrange(noofjobs):
    job = "job"+str(jobno)
    os.chdir("PandoraCondor/work/")
    os.chdir(job)
    newname = job+"d.root"
    if (os.path.exists("outputd.root")):
        os.rename("outputd.root",newname)
        destination = "../../../"+newname
        shutil.move(newname, destination)
        os.chdir("../../../")
    else:
        minus = minus + 1
        os.chdir("../../../")


noofjobs = noofjobs - minus

for jobno in xrange(noofjobs):
    job = "job"+str(jobno)+"d.root"
    chain.Add(job)

tree = chain


file.Write()
file.Close()

#--------------------------------------------------------------------

wasright = []
for iev in xrange(tree.GetEntries()):
    tree.GetEntry(iev)
    wasrightval = getattr(tree, "wasright")
    wasright.append(wasrightval)

wasrightarray = np.array(wasright)
print wasrightarray.size

totallen = len(wasright)

right = []
wrong = []

for x in range(wasrightarray.size):
    var = wasrightarray[x]
    if (var == 1):
        right.append(1)
    if (var == 0):
        wrong.append(1)


rightlen = float(len(right))
wronglen = float(len(wrong))
print "right ", rightlen
print "wrong ", wronglen
print "total ", totallen

rightness = rightlen/totallen
print "The percentage of times the most likely neutrino was correctly identified is ", round(rightness*100, 2)

#--------------------------------------------

is0nan = []
for iev in xrange(tree.GetEntries()):
    tree.GetEntry(iev)
    val = getattr(tree, "isa0nan")
    is0nan.append(val)

is0nanarray = np.array(is0nan)
print is0nanarray.size

totallen0 = len(is0nan)

yes0nan = []
not0nan = []

for x in range(is0nanarray.size):
    var = is0nanarray[x]
    if (var == 1):
        yes0nan.append(1)
    if (var == 0):
        not0nan.append(1)


yeslen = float(len(yes0nan))
nolen = float(len(not0nan))
print "is 0/nan ", yeslen
print "not 0/nan ", nolen
print "total ", totallen0

print wasrightarray
print is0nanarray

#------------------------------------
rightnot0 = []
wrongnot0 = []

for x in range(wasrightarray.size):
    var = wasrightarray[x]
    var2 = is0nanarray[x]
    if (var == 1 and var2 == 0):
        rightnot0.append(1)
    if (var == 0 and var2 == 0):
        wrongnot0.append(1)

rightnot0len = float(len(rightnot0))
wrongnot0len = float(len(wrongnot0))
print "right but not 0 = ", rightnot0len
print "wrong but not 0 = ", wrongnot0len
