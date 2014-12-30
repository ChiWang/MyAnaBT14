>+
>+ input DAMPE_Bgo_hits, AMS root file from University Perugia, Ancillary root file from bari
>+
>+


0.  install DMPSW

1.  run examples (after step 1)
root -l  Run.C
test()

2.  Combine two sides file into one root (only for AMS SPS)
root -l Run.C
CombineData::AMSSPS("file_from_side0_run002","file_from_side1_run002");

3. Combine DAMPE, AMS, ANC
CombineData::DAMPE_AMS_ANC("file_DAMPE","file_AMS","file_ANC");


>+
>+   could set where to read data in root interpator by using the path if the lib is already loaded:
>+  Conf::Path="./Data/"
>+   under this path must have directories: Root_Data_side0, Root_Data_side1; and the path name must end with /.  The default path is "./Data/"
>+


