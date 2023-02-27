find_segment_of ->  verifica din ce segment al executanbilului face parte pagina nemapata

if (!segment|si->si_code==SEGV_ACCERR) -> verific daca segmentul exista si dc exista verific prin SEGV_ACCERR dc pagina este mapata la o zona de memorie valida sau la care procesul are permisiune 

copy_into -> se obtine adresa de memorie mapata prin apelul  mmap ulterior continutul paginii executabilului fiind copiat in aceasta zona de memorie

mm_read -> citeste efectiv din fd executabilului bitii