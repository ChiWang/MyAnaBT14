>+
>+ input is the output of Perugia Decode package.
>+   only used for combining SPS 2 root file which from side_0 and side_1
>+
>+


0.  install DMPSW

1.  run examples (after step 1)
root -l  Run.C
test()

3.  Combine two sides file into one root (only for SPS)
root -l Run.C
CombineData::AMSSPS("file_from_side0_run002","file_from_side1_run002");


>+
>+   could set where to read data in root interpator by using the path if the lib is already loaded:
>+  Conf::Path="./Data/"
>+   under this path must have directories: Root_Data_side0, Root_Data_side1; and the path name must end with /.  The default path is "./Data/"
>+


