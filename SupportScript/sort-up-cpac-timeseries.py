# -*- coding: utf-8 -*-
"""
Created on Wed Nov  9 16:33:41 2016

@author: Tian
"""

# output format:
# <roi>/<subject id>/<session name>-<scan name>.1D
# eg. aal/28716/session_1-rest_1.1D
# eg.

import sys,os,re
import shutil
import datetime

ROI_LIST={"aal_mask_pad":'aal', "CC200":"cc200", "CC400":"cc400", "ez_mask_pad":'ez',
          "ho_mask_pad":'ho', "rois_3mm":'3mm', "tt_mask_pad":'tt'}
ROI_LIST_LEN=len(ROI_LIST)

def _formatFolderName(folder):
    if folder[-1]!='/' and folder[-1]!='\\':
        folder=folder+'/'
    return folder

def _checkFNwithPattern(name,parent,pattern):
    return os.path.exists(parent+'/'+name) and re.match(pattern,name)

# parseSession_XXX returns (subject id, session name)

_pat_session_ABIDE=r"(\d+)_([^/\\]+)";
def parseSession_ABIDE(s):
    r=re.match(_pat_session_ABIDE,s)
    if not r: return None
    return (r.group(1), r.group(2))

_pat_session_ADNI=r"(\d+)_S_(\d+)[^\d]*";
def parseSession_ADNI(s):
    r=re.match(_pat_session_ADNI,s)
    if not r: return None
    return (r.group(2), r.group(2))
    #return (r.group(1)+r.group(2), r.group(1)+r.group(2))

# parseScan_XXX returns (scan name, raw scan id)

_pat_scan_ABIDE=r"_scan_(.+?)_(\d+)"
def parseScan_ABIDE(s):
    r=re.match(_pat_scan_ABIDE,s)
    if not r: return None
    return (r.group(1), r.group(2))

_pat_scan_ADNI=r"_scan_scan(\d+)_.*"
def parseScan_ADNI(s):
    r=re.match(_pat_scan_ADNI,s)
    if not r: return None
    return ('scan', r.group(1))

_pat_scan_ANY=r"_scan_(.*?)(\d+)"
def parseScan_ANY(s):
    r=re.match(_pat_scan_ANY,s)
    if not r: return None
    return (r.group(1), r.group(2))


def main(inFolder, outFolder, dataset, globalOpt):
    inFolder=_formatFolderName(inFolder)
    outFolder=_formatFolderName(outFolder)
    pat_decimal='\d(\.\d+)?';
    #pat_session_ABIDE=r"(\d+)_[^/]+";
    #pat_session_ADNI=r"\d+_S_(\d+)[^\d]*";
    #fun_session
    pat_ts=r"roi_timeseries"
    #pat_scan_ABIDE=r"_scan_(rest)_(\d+)_.*"
    #pat_scan_ADNI=r"_scan_(scan)(\d+)_.*"
    #pat_scan_ANY=r"_scan_.*"
    #fun_scan
    pat_csf=r"_csf_threshold_"+pat_decimal
    pat_gm=r"_gm_threshold_"+pat_decimal
    pat_wm=r"_wm_threshold_"+pat_decimal
    pat_cor=r"_compcor_ncomponents_\d+_selector_pc1\d\.linear\d\.wm\d\.global"+globalOpt+r"\.motion\d\.quadratic\d\.gm\d\.compcor\d\.csf\d"
    pat_bp=r"_bandpass_freqs_"+pat_decimal+r"\."+pat_decimal

    if dataset=='ABIDE':
        #pat_session=pat_session_ABIDE
        #pat_scan=pat_scan_ABIDE
        fun_session=parseSession_ABIDE
        fun_scan=parseScan_ABIDE
    elif dataset=='ADNI':
        #pat_session=pat_session_ADNI
        #pat_scan=pat_scan_ANY
        fun_session=parseSession_ADNI
        fun_scan=parseScan_ADNI

    for (k,v) in ROI_LIST.items():
        fn=os.path.join(outFolder,v)
        if not os.path.exists(fn):
            os.makedirs(fn)

    nameRecord={}

    cntSsn=0
    cntSsnVld=0
    for session in os.listdir(inFolder):
        #<session id>
        r=fun_session(session)
        if not r:
            print(session+' invalid session name')
            continue
        (subId, ssnName)=r
        pi=inFolder+session+'/'+pat_ts+'/'
        #<session id>/roi_timeseries
        if not os.path.exists(pi):
            print(session+' no content inside')
            continue
        cntSsn+=1
        print(session)
        cntScan=0
        for scan in os.listdir(pi):
            #<session id>/roi_timeseries/_scan*
            r2=fun_scan(scan)
            if not r2:
                continue
            (scnName,scnId)=r2
            #outFn=ssnId+'_'+scnName+'_'+scnId+'.1D'
            #outFn='scan_'+str(cntScan)+'.1D'
            outFn=scnName+'_'+scnId+'.1D'
            pii=pi+scan+'/'
            lcsf=os.listdir(pii)
            #<session id>/roi_timeseries/_scan*/_csf*
            if len(lcsf)==0 or not re.match(pat_csf,lcsf[0]):
                continue
            pii=pii+lcsf[0]+'/'
            lgm=os.listdir(pii)
            #<session id>/roi_timeseries/_scan*/_csf*/_gm*
            if len(lgm)==0 or not re.match(pat_gm,lgm[0]):
                continue
            pii=pii+lgm[0]+'/'
            lwm=os.listdir(pii)
            #<session id>/roi_timeseries/_scan*/_csf*/_gm*/_wm*
            if len(lwm)==0 or not re.match(pat_wm,lwm[0]):
                continue
            pii=pii+lwm[0]+'/'
            lcor=os.listdir(pii)
            #<session id>/roi_timeseries/_scan*/_csf*/_gm*/_wm*/_compcor*global(0/1)*
            cntCor=0
            for fnCor in lcor:
                if not re.match(pat_cor,fnCor):
                    continue
                lib=os.listdir(pii+fnCor)
                #<session id>/roi_timeseries/_scan*/_csf*/_gm*/_wm*/_compcor*global(0/1)*/_bandpass*
                if len(lib)==0 or not re.match(pat_bp,lib[0]):
                    continue
                nameRecord[subId+"-"+str(cntScan)]=session+" - "+scan
                cntMask=0
                #print('  '+scan,end=' ') #only work on python 3
                sys.stdout.write('  '+scan+' with ')
                usedRoi=[]
                for mask in os.listdir(pii+fnCor+'/'+lib[0]):
                    #<session id>/roi_timeseries/_scan*/_csf*/_gm*/_wm*/_compcor*global(0/1)*/_bandpass*/_mask*
                    roi=re.sub('^_mask_','',mask)
                    #print(roi, mask+'/'+'roi_'+roi+'.1D')
                    pim=pii+fnCor+'/'+lib[0]+'/'+mask+'/'+'roi_'+roi+'.1D'
                    if os.path.exists(pim):
                        po=outFolder+ROI_LIST[roi]+'/'+subId+'/'
                        usedRoi.append(ROI_LIST[roi])
                        if not os.path.exists(po):
                            os.mkdir(po)
                        shutil.copy2(pim,po+outFn)
                        cntMask+=1
                #print(str(cntMask)+ ' ROI(s) '+str(usedRoi))
                if cntMask==ROI_LIST_LEN:
                    print(str(cntMask)+' ROI(s)')
                else:
                    print(str(cntMask)+' ROI(s) : '+str(usedRoi))
                if cntMask!=0:
                    cntCor+=1
            if cntCor!=0:
                cntScan+=1
        if cntScan==0:
            print('  No valid scans!')
        else:
            cntSsnVld+=1
    print(str(cntSsn)+' session(s) and '+str(cntSsnVld)+' valid session(s)')
    print('writing name mapping...')
    fout=open(outFolder+'nameMapping.txt','a')
    fout.write(str(datetime.datetime.now()))
    fout.write('\n')
    for (k,v) in nameRecord.items():
        fout.write(k+'\t:\t'+v+'\n')
    fout.close()


#opt/ABIDEII_CPAC_series_holo/pipeline_abide_rerun_II_4CORE__freq-filter/50051_baseline/roi_timeseries/_scan_rest_1_rest/_csf_threshold_0.96/_gm_threshold_0.7/_wm_threshold_0.96/_compcor_ncomponents_5_selector_pc10.linear1.wm0.global1.motion1.quadratic1.gm0.compcor1.csf0/_bandpass_freqs_0.01.0.1$
#output/pipeline_ADNI_pipeline/014_S_4615_MoCoSeries/roi_timeseries/_scan_scan0_ASL_PERFUSION/_csf_threshold_0.96/_gm_threshold_0.7/_wm_threshold_0.96/_compcor_ncomponents_5_selector_pc10.linear1.wm0.global0.motion1.quadratic1.gm0.compcor1.csf0/_bandpass_freqs_0.01.0.08/_mask_aal_mask_pad

#subjects:
#29433_session_1  28716_session_1  28949_session_1  28983_session_1
#002_S_0413_Resting_State_fMRI  002_S_4799_Resting_State_fMRI  013_S_4616_Resting_State_fMRI

#scans:
#_scan_rest_1_rest
#_scan_scan0_S111991  _scan_scan1_S150694  _scan_scan2_S189129
#_compcor_ncomponents_5_selector_pc10.linear1.wm0.global0.motion1.quadratic1.gm0.compcor1.csf0  _compcor_ncomponents_5_selector_pc10.linear1.wm0.global1.motion1.quadratic1.gm0.compcor1.csf0

#_mask_aal_mask_pad  _mask_CC200  _mask_CC400  _mask_ez_mask_pad  _mask_ho_mask_pad  _mask_rois_3mm  _mask_tt_mask_pad
#roi_aal_mask_pad.1D  roi_aal_mask_pad.csv  roi_aal_mask_pad.npz  roi_aal_mask_pad.txt
#roi_CC200.1D  roi_CC200.csv  roi_CC200.npz  roi_CC200.txt
#roi_rois_3mm.1D  roi_rois_3mm.csv  roi_rois_3mm.npz  roi_rois_3mm.txt

#30151_session_1
#
#50002_baseline
#50002_followup_1
#50002_followup_2
#



if __name__=='__main__':
    if len(sys.argv)<4 or len(sys.argv)>5:
        print('Usage: <in-folder> <out-folder> <dataset> [global-opt=0/1]')
        print("  dataset: ABIDE, ADNI. It determines the name patten.")
        print("  The in-folder should directly contain the subjects")
        exit()
    inFolder=sys.argv[1];
    outFolder=sys.argv[2];
    dataset=sys.argv[3];
    if dataset not in {'ABIDE', 'ADNI'}:
        raise Exception("do not support given dataset: "+dataset)
    globalOpt='0'
    if len(sys.argv)>4:
        globalOpt=sys.argv[4]
    main(inFolder, outFolder, dataset, globalOpt)

