-- 
-- Find X$KSUSE.KSUSEOPC and  X$KSUSE.KSUSETIM given the session SID
-- To be used together with ora_kstackprofiler
--
-- Luca.Canali@cern.ch, November 2015
--

set serveroutput on
set verify off

prompt This script finds the OS pid and address of X$KSUSE.KSUSEOPC and X$KSUSE.KSUSETIM 
prompt for a given Oracle session. To be used together with ora_kstackprofiler
prompt Run as user SYS
prompt
accept SID NUMBER PROMPT "Enter Oracle SID to be investigated: " 

declare
  v_pid number;
  v_ksusetim_offset number;
  v_ksuseopc_offset number;
  v_saddr number;

begin

  select p.spid into v_pid
  from v$session s, v$process p
  where p.addr=s.paddr and s.sid = &SID;

  select c.kqfcooff into v_ksuseopc_offset 
  from x$kqfco c, x$kqfta t
  where t.indx = c.kqfcotab
        and t.kqftanam = 'X$KSUSE'
        and c.kqfconam = 'KSUSEOPC';
		
  select c.kqfcooff into v_ksusetim_offset 
  from x$kqfco c, x$kqfta t
  where t.indx = c.kqfcotab
        and t.kqftanam = 'X$KSUSE'
        and c.kqfconam in 'KSUSETIM';
  
  select to_number(saddr,'xxxxxxxxxxxxxxxx') into v_saddr from v$session where sid = &SID;

  dbms_output.put_line('OS pid = '||to_char(v_pid)||', ksuseopc = '|| to_char(v_saddr+v_ksuseopc_offset) ||
                       ', ksusetim = '|| to_char(v_saddr+v_ksusetim_offset));
 
exception
  when NO_DATA_FOUND then
        dbms_output.put_line('The given SID does not exist or details cannot be found');      

end;
/
 
 
