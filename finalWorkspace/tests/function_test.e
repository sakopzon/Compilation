UJUMP 35
SUBTI I1 I1 0
COPYI I3 1
ADD2I I1 I2 1
STORI I3 I1 0
RETRN
SUBTI I1 I1 1
LOADI I3 I2 0
PRNTI I3
STORI I3 I1 0
ADD2I I1 I1 -1
STORI I0 I1 0
STORI I2 I1 -1
SUBTI I1 I1 3
COPYI I2 I1
SUBTI I1 I1 1
JLINK 2
LOADI I4 I1 0
LOADI I2 I1 1
LOADI I0 I1 2
ADD2I I1 I1 3
LOADI I3 I1 0
ADD2I I1 I1 1
STORI I4 I2 -1
LOADI I5 I2 -1
LOADI I6 I2 0
ADD2I I7 I5 I6
STORI I7 I2 -1
LOADI I8 I2 -1
PRNTI I8
LOADI I9 I2 -1
ADD2I I1 I2 1
STORI I9 I1 0
RETRN
COPYI I1 999
COPYI I2 999
SUBTI I1 I1 1
COPYI I3 5
STORI I3 I1 0
ADD2I I1 I1 -1
STORI I0 I1 0
STORI I2 I1 -1
SUBTI I1 I1 3
COPYI I2 I1
STORI I3 I1 0
SUBTI I1 I1 1
JLINK 7
LOADI I4 I1 0
LOADI I2 I1 1
LOADI I0 I1 2
ADD2I I1 I1 3
LOADI I3 I1 0
ADD2I I1 I1 1
STORI I4 I2 0
LOADI I5 I2 0
PRNTI I5
HALT