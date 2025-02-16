#ifndef _NHOPPARTITION_HPP_
#define _NHOPPARTITION_HPP_

//#include "nHop_kernel.hpp"
#include "nHopPartition.h"
#include "host.hpp"
//extern CTimeModule<unsigned long> gtimer;

template <class T>
int FindSegmentPoint( // return nv_end_start
    int num_chnl_par,
    CSR<T>& src,
    T limit_nv,
    T limit_ne,
    T nv_start) {
    T nv_end_start; // return

    // Calculat reference nv and ne
    T nv_ref = (src.NV + num_chnl_par - 1) / num_chnl_par;
    T ne_ref = (src.NE + num_chnl_par - 1) / num_chnl_par;
    if (nv_ref > limit_nv) nv_ref = limit_nv;
    if (ne_ref > limit_ne) ne_ref = limit_ne;//fixed limit_nv to limit_ne

    nv_end_start = nv_start + nv_ref;
    if (nv_end_start > src.NV) nv_end_start = src.NV;

    T ne_start = src.offset[nv_start];
    T ne_end = src.offset[nv_end_start];

    while ((ne_end - ne_start) > limit_ne) {
        nv_end_start--;
        ne_end = src.offset[nv_end_start];
    }
    return nv_end_start;
}

template <class T>
int CSRPartition_average( // return the real number of partition
    int num_chnl_par,
    CSR<T>& src,
    T limit_nv,
    T limit_ne,
    CSR<T>* des[]) {
    if (num_chnl_par <= 0 || !src.isInit()) return -1;

    T nv_start = 0;
    T ne_start = 0;
    int cnt_par = 0;
    do {
        T nv_end_start = FindSegmentPoint<T>(num_chnl_par, src, limit_nv, limit_ne, nv_start);
        T ne_end = src.offset[nv_end_start];
        des[cnt_par] = new CSR<T>;
        des[cnt_par]->Init(nv_start, nv_end_start, src);
        // if(!cnt_par){
        //     std::cout << "id=" << cnt_par << " offsetTable=" << nv_start << " indexTable=" << src.offset[nv_start] << std::endl;
        //     std::cout << "id=" << cnt_par+1 << " offsetTable=" << nv_end_start << " indexTable=" << src.offset[nv_end_start] << std::endl;
        // }else{
        //     std::cout << "id=" << cnt_par+1 << " offsetTable=" << nv_end_start << " indexTable=" << src.offset[nv_end_start] << std::endl;
        // }
        nv_start = nv_end_start;
        cnt_par++;
    } while (nv_start != src.NV);
    return cnt_par;
}

template <class T>
int CSRPartition_average_onOnekernel( // return the real number of partition
    int num_chnl_par,
    CSR<T>& src,
    T limit_nv,
    T limit_ne,
    CSR<T>* des[]) {
    if (num_chnl_par <= 0 || !src.isInit()) return -1;

    T nv_ref = (src.NV + num_chnl_par - 1) / num_chnl_par;

    T nv_start = 0;
    T ne_start = 0;
    int cnt_par = 0;
    do {
        T nv_end_start = nv_start + nv_ref;
        if (nv_end_start > src.NV) nv_end_start = src.NV;
        des[cnt_par] = new CSR<T>;
        des[cnt_par]->Init(nv_start, nv_end_start, src);

        nv_start = nv_end_start;
        cnt_par++;
    } while (nv_start != src.NV);
    return cnt_par;
}

template <class T>
int FindPar(T v, int num_par, T* tab_startV) {
    for (int i_par = 0; i_par < num_par; i_par++) {
        if (i_par == num_par - 1)
            return i_par;
        else if (v >= tab_startV[i_par + 1])
            continue;
        else
            return i_par;
    }
    printf("ERROR: FindPar can't find the partition ID, the content of tab_startV may have error\n");
    for (int i = 0; i < num_par; i++) printf("%d: startV=%d\n", i, tab_startV[i]);
    return -1;
}
template <class T>
int SelectParID(T v, int num_par, T* tab_startV, int* tab_copy, int* state_copy) {
    int i_par = FindPar<T>(v, num_par, tab_startV);
    int ncp = tab_copy[i_par];
    if (ncp == 1) return i_par;    // Only one copy, so the i_par is the final parition index
    int st_rr = state_copy[i_par]; // st_rr: Round-Robin state register, including states from 0, 1,.., to ncp-1;
    int ret = st_rr * num_par +
              i_par; //[0, 1,...., num_par-1], then to [0, 1,...., num_par-1]........[0, 1,... num_dev % num_par]
    // Updating Round-Robin state register
    if (st_rr < ncp - 1)
        state_copy[i_par]++;
    else
        state_copy[i_par] = 0;
    return ret;
}

template <class T>
bool CheckRange_v(T v, T V_start, T V_end_1) {
    if (v < V_start) {
        printf("v(%d) < V_start(%d)\n", v, V_start);
        return false;
    }
    if (v >= V_end_1) {
        printf("v(%d) >= V_end plus 1(%d)\n", v, V_end_1);
        return false;
    }
    return true;
}

/***********************************/
// partition data for hop channel mothed
/***********************************/
template <class T>
T* HopChnl<T>::LookUp(
    int* degree,
    T v) // For CPU similation, no need to created real data structure like HopPack which will take too more memory
{
    assert(pCSR);
    isVinRange(v);
    assert(v >= this->pCSR->V_start);
    assert(v < this->pCSR->V_end_1);
    T off1 = getOffset(v);
    T off2 = getOffset(v + 1);
    *degree = off2 - off1;
    return this->pCSR->index + (off1 - indexShift);
}

template <class T>
void HopChnl<T>::LookUp(
    AccessPoint<T>* accp,
    T v) // For CPU similation, no need to created real data structure like HopPack which will take too more memory
{
    accp->p_idx = LookUp(&accp->degree, v);
}

template <class T>
void HopChnl<T>::LookUp(AccessPoint<T>* accp, HopPack<T>* hpk) // For CPU similation, no need to created real data
                                                               // structure like HopPack which will take too more memory
{
    T v;
    if (hpk->hop == 0)
        v = hpk->des;
    else
        v = hpk->idx;
    accp->p_idx = LookUp(&accp->degree, v);
}

template <class T>
T HopChnl<T>::LookUp(T* des, int size, T v) { // Return the real degree;
    AccessPoint<T> accp;
    LookUp(&accp, v);
    if (size > accp.degree) size = accp.degree;
    for (int i = 0; i < size; i++) des[i] = accp.p_idx[i];
    return accp.degree;
}

/***********************************/
// packbuff mothed
/***********************************/
template <class T>
void PackBuff<T>::ShowInfo_buff(bool isReturn) {
    printf("%9ld used (%2.1f%% of %9ld) p_write:%9ld, p_read:%9ld", GetNum(), Ratio_used(), size, p_write, p_read);
    if (isReturn) printf("\n");
}
template <class T>
void PackBuff<T>::print() {
    for (int i = 0; i < GetNum(); i++) {
        long pi = (p_read + i) % size;
        HopPack<T>* ppk = pBuff + pi;
        ppk->print(i);
    }
}
template <class T>
void PackBuff<T>::ShowInfo_buff(const char* nm, bool isReturn) {
    printf("%s:%6ld(%2.1f%%) \t", nm, GetNum(), Ratio_used());
    if (isReturn) printf("\n");
}
template <class T>
int PackBuff<T>::InitBuff(long size_buff) {
    size = size_buff;
    pBuff = (HopPack<T>*)malloc(sizeof(HopPack<T>) * size);
    assert(pBuff);
    p_write = p_read = 0;
    Empty = true;
    return 0;
}
template <class T>
void PackBuff<T>::ResetBuff() {
    p_write = p_read = 0;
    Empty = true;
}

template <class T>
int PackBuff<T>::push(AccessPoint<T>* p_accp, HopPack<T>* p_hpk) { // return how many package stored
    if (isFull(p_accp->degree)) {
        printf("ERROR: BUFF FULL!!!\n");
        return 0;
    }
    T idx_old = p_hpk->idx;
    for (int i = 0; i < p_accp->degree; i++) {
        p_hpk->idx = p_accp->p_idx[i];
        memcpy((void*)(pBuff + p_write), (void*)p_hpk, sizeof(HopPack<T>));
        p_write++;
        if (p_write == size) p_write = 0;
    }
    p_hpk->idx = idx_old; // recover the orignal value
    Empty = false;
    return p_accp->degree;
}

template <class T>
int PackBuff<T>::push(HopPack<T>* p_hpk) {
    if (isFull()) {
        printf("ERROR: BUFF FULL!!!\n");
        return 0;
    }
    memcpy((void*)(pBuff + p_write), (void*)p_hpk, sizeof(HopPack<T>));
    p_write++;
    Empty = false;
    if (p_write == size) p_write = 0;
    return 1;
}

template <class T>
int PackBuff<T>::pop(HopPack<T>* p_hpk) {
    if (isEmpty()) return 0;
    memcpy((void*)p_hpk, (void*)(pBuff + p_read), sizeof(HopPack<T>));
    p_read++;
    if (p_read == size) p_read = 0;
    if (p_read == p_write) Empty = true;
    return 1;
}

template <class T>
void PackBuff<T>::SavePackBuff(long num, const char* name){
    FILE* fp=fopen(name, "w");
    fprintf(fp,"%ld\n", num);
    for(int i=0; i<num; i++){
        fprintf(fp, "%ld %ld %ld \n", pBuff[i].src, pBuff[i].des, pBuff[i].idx);
    }
    fclose(fp);
}

/***********************************/
// HopKernel mothed
/***********************************/
// Logic function for judging a package content; There can be a lot of logic funtions for different hop-based
// application
template <class T>
bool HopKernel<T>::Judge_equal(HopPack<T>& pkg) {
    if (pkg.des == pkg.idx)
        return true;
    else
        return false;
}

template <class T>
void HopKernel<T>::InitBuffs(long sz_in, long sz_out, long sz_pp, long sz_agg) {
    p_buff_in = new (PackBuff<T>);   //->InitBuff(sz_in);
    p_buff_out = new (PackBuff<T>);  //->InitBuff(sz_out);
    p_buff_ping = new (PackBuff<T>); //->InitBuff(sz_pp);
    p_buff_pang = new (PackBuff<T>); //->InitBuff(sz_pp);
    p_buff_agg = new (PackBuff<T>);  //->InitBuff(sz_agg);
    p_buff_in->InitBuff(sz_in);
    p_buff_out->InitBuff(sz_out);
    p_buff_ping->InitBuff(sz_pp);
    p_buff_pang->InitBuff(sz_pp);
    p_buff_agg->InitBuff(sz_agg);
    p_buff_pp[0] = p_buff_ping;
    p_buff_pp[1] = p_buff_pang;
}

template <class T>
void HopKernel<T>::LookUp(AccessPoint<T>* p_accp, HopPack<T>* p_hpk) // For CPU similation, no need to created real data
                                                                     // structure like HopPack which will take too more
                                                                     // memory
{
    T v;
    if (p_hpk->hop == 0)
        v = p_hpk->des;
    else
        v = p_hpk->idx;
    int num_chnl_knl_par = num_chnl_par < num_chnl_knl ? num_chnl_par : num_chnl_knl;
    int id_ch = SelectParID<T>(v, num_chnl_knl_par, tab_disp_ch, tab_copy_ch, state_copy_ch);
    assert(id_ch < num_chnl_knl);
    HopChnl<T>* pch = &(channels[id_ch]);
    pch->LookUp(p_accp, p_hpk);
}

template <class T>
void HopKernel<T>::InitCore(CSR<T>* par_chnl_csr[], int start, int num_ch_knl, int num_ch_par) {
    start_chnl = start;
    this->num_chnl_knl = num_ch_knl;
    this->num_chnl_par = num_ch_par;
    int n_cpy = (num_ch_knl + num_ch_par - 1) / num_ch_par;
    for (int c = 0; c < n_cpy; c++) {
        for (int i = 0; i < num_ch_par; i++) {
            int id_ch = i + num_ch_par * c;
            if (id_ch >= num_ch_knl) break;
            channels[id_ch].Init(par_chnl_csr[start_chnl + i]);
            tab_copy_ch[i] = (c == 0) ? 1 : tab_copy_ch[i] + 1;
            tab_disp_ch[i] = par_chnl_csr[start_chnl + i]->V_start;
            state_copy_ch[i] = 0;
        }
    }
}

template <class T>
void HopKernel<T>::InitCore(CSR<T>* par_chnl_csr[],
                            int start,
                            int num_ch_knl,
                            int num_ch_par,
                            int num_knl_p,
                            int id_knl_u,
                            T* tab_disp_k,  //[MAX_NUM_KNL_PAR];
                            int* tab_copy_k //[MAX_NUM_KNL_PAR];
                            ) {
    start_chnl = start;
    this->num_chnl_knl = num_ch_knl;
    this->num_chnl_par = num_ch_par;
    this->num_knl_par = num_knl_p;
    this->id_knl_used = id_knl_u;
    int n_cpy = (num_ch_knl + num_ch_par - 1) / num_ch_par;
    for (int c = 0; c < n_cpy; c++) {
        for (int i = 0; i < num_ch_par; i++) {
            int id_ch = i + num_ch_par * c;
            if (id_ch >= num_ch_knl) break;
            channels[id_ch].Init(par_chnl_csr[start_chnl + i]);
            tab_copy_ch[i] = (c == 0) ? 1 : tab_copy_ch[i] + 1;
            tab_disp_ch[i] = par_chnl_csr[start_chnl + i]->V_start;
            state_copy_ch[i] = 0;
        }
    }

    for (int i = 0; i < this->num_knl_par; i++) {
        this->tab_disp_knl[i] = tab_disp_k[i];
        this->tab_copy_knl[i] = tab_copy_k[i];
        this->tab_state_knl[i] = 0;
    }
}

template <class T>
int HopKernel<T>::SelectChnlParID(T v) {
    int i_par;
    for (i_par = 0; i_par < num_chnl_par; i_par++) {
        if (i_par == num_chnl_par - 1) break;
        if (this->tab_disp_ch[i_par + 1])
            continue;
        else
            break;
    }
    int ncp = GetNumChnlCopy();
    if (ncp == 1) return i_par;
    int st_rr = state_copy_ch[i_par]; // 0, 1,..ncp-1;
    int ret = st_rr * num_chnl_par;
    if (st_rr < ncp - 1)
        state_copy_ch[i_par]++;
    else
        state_copy_ch[i_par] = 0;
}

template <class T>
void HopKernel<T>::ShowHopKernelInfo(int id_core) {
    printf("HopKernel(%d) to cover %d of %d channel partitions, including Par_ID_ch:  ", id_core, num_chnl_knl,
           num_chnl_par);
    for (int i = 0; i < this->num_chnl_knl; i++) {
        printf(" %4d,", GetChnlParID(i)); // start_chnl + i%this->num_chnl_par);
    }
    printf("\n");
}
template <class T>
void HopKernel<T>::ShowDispTab_intra(int id_core) {
    printf("Updating Channel Dispatching Table in kernel(%d): \n", id_core);
    printf("-------------------V_start------Copy---- \n");
    int size_tab = num_chnl_par < num_chnl_knl ? num_chnl_par : num_chnl_knl;
    for (int i = 0; i < size_tab; i++) {
        printf("Par_ID_ch:%3d:  %8d,      %3d\n", i, tab_disp_ch[i], tab_copy_ch[i]);
    }
}

/***********************************/
// PartitionHop mothed
/***********************************/
template <class T>
int PartitionHop<T>::NumCpy_knl(int ParID_knl) {
    int rem = num_knl_used % num_knl_par;
    if (rem == 0) return NumCpy_knl();
    if (ParID_knl % num_knl_par > num_knl_used % num_knl_par)
        return NumCpy_knl() - 1;
    else
        return NumCpy_knl();
}

template <class T>
void PartitionHop<T>::FreePar() {
    for (int i = 0; i < num_chnl_par; i++) free(par_chnl_csr[i]);
    num_chnl_par = 0;
}
template <class T>
void PartitionHop<T>::FreeKnl() {
    for (int i = 0; i < num_knl_used; i++) free(hopKnl[i]);
    num_knl_used = 0;
}

template <class T>
int PartitionHop<T>::GetNumKnlCopy(int id_kp) { // Return how many copies for this kernel partition
    int min = this->num_knl_used / this->num_knl_par;
    int rem = this->num_knl_used % this->num_knl_par;
    int rem_id = (id_kp) % this->num_knl_par;
    if (rem_id >= rem)
        return min;
    else
        return min + 1;
}

template <class T>
void PartitionHop<T>::syncDispTab_inter() {
    printf("Updating Kernel Dispatching Table: \n");
    printf("-------------------V_start------Copy---- \n");
    for (int ikp = 0; ikp < this->num_knl_par; ikp++) {
        this->tab_disp_knl[ikp] = this->par_chnl_csr[ikp * this->num_chnl_knl]->V_start;
        this->tab_copy_knl[ikp] = this->GetNumKnlCopy(ikp);
        printf("Par_ID_knl:%3d:  %8d,      %3d\n", ikp, this->tab_disp_knl[ikp], this->tab_copy_knl[ikp]);
    }
}
template <class T>
void PartitionHop<T>::syncDispTab_intra(HopKernel<T>* pKnl, // updating:
                                        // T* tab_disp_ch,//output
                                        // int* tab_copy_ch,//output
                                        int id_knl_used) {
    int idk_par = id_knl_used % this->num_knl_par;
    int idc_off = idk_par * this->num_chnl_knl;
    printf("Updating Channel Dispatching Table in kernel(%d): \n", id_knl_used);
    printf("-------------------V_start------Copy---- \n");
    for (int icp = 0; icp < this->num_chnl_knl; icp++) {
        pKnl->tab_disp_ch[icp] = this->par_chnl_csr[idc_off + icp]->V_start;
        pKnl->tab_copy_ch[icp] = this->hopKnl[id_knl_used]->GetNumChnlCopy();
        printf("Par_ID_ch:%3d:  %8d,      %3d\n", icp, pKnl->tab_disp_ch[icp], pKnl->tab_copy_ch[icp]);
    }
}
template <class T>
void PartitionHop<T>::ShowDevPar() {
    for (int i = 0; i < this->num_knl_used; i++) this->hopKnl[i]->ShowHopKernelInfo(i);
}
// template <class T>
// int PartitionHop<T>::Deploykernel(int num_knl_used_in, HopKernel<T>* p_knl){

//     }
template <class T>
int PartitionHop<T>::FindChnlInKnl( // give a channel parition ID, the function return how many and which kernel and
                                    // which hop-channel in the kernel deployed the channel paritition with the ID
    int* p_id_knl, // output: kernels' id in a stream
    int* p_no_ch,  // output: which channel in the kernel used for store the 'id_ch_par'
    int id_ch_par  // intput: Channel-paritition ID
    ) {
    int ret = 0; // How many channels in this kernel
    for (int d = 0; d < this->num_knl_used; d++) {
        for (int c = 0; c < this->num_chnl_knl; c++) {
            int ich_d_c = this->hopKnl[d]->GetChnlParID(c);
            if (ich_d_c == id_ch_par) {
                p_id_knl[ret] = d;
                p_no_ch[ret] = c;
                ret++;
            }
        }
    }
    return ret;
}
template <class T>
void PartitionHop<T>::ShowInfo() {
    for (int i = 0; i < num_chnl_par; i++) {
        par_chnl_csr[i]->ShowInfo("Par_ID_ch: ", i, limit_v_b, limit_e_b);
        printf("\n");
    }
}

template <class T>
void PartitionHop<T>::ShowChnlPar() {
    for (int i = 0; i < num_chnl_par; i++) {
        int ids[MAX_NUM_KNL_USED];
        int idc[MAX_NUM_KNL_USED];
        int num_copy = FindChnlInKnl(ids, idc, i);
        if ((i) % num_chnl_knl == 0) {
            printf(
                "---------------------------kernel Partition %d with %d channel paritions "
                "---------------------------------------and has %d copies on ",
                i / num_chnl_knl, num_chnl_knl, num_copy);
            for (int d = 0; d < num_copy; d++) printf("Knl(%d) ", ids[d]);
            printf("-------------------------\n");
        }
        par_chnl_csr[i]->ShowInfo("Par_ID_ch: ", i, limit_v_b, limit_e_b);
        printf("The channel has %d copy (Knl, Ch):", num_copy);
        for (int d = 0; d < num_copy; d++) printf("(%d,%d), ", ids[d], idc[d]);
        printf("\n");
    }
}
template <class T>
void PartitionHop<T>::ShowInfo_buff_in() {
    printf("\n");
    printf("Report of loading input: \n");
    for (int i = 0; i < this->num_knl_used; i++) {
        printf("ID_KERNEL_USED:%d  ", i);
        this->hopKnl[i]->p_buff_in->ShowInfo_buff();
    }
}
template <class T>
void PartitionHop<T>::ShowInfo_buff_out() {
    printf("\n");
    printf("Report of output: \n");
    for (int i = 0; i < this->num_knl_used; i++) {
        printf("ID_KERNEL_USED:%d  ", i);
        this->hopKnl[i]->p_buff_out->ShowInfo_buff();
    }
}
template <class T>
void PartitionHop<T>::ShowInfo_buff_agg() {
    printf("\n");
    printf("Report of aggregation buff: \n");
    for (int i = 0; i < this->num_knl_used; i++) {
        printf("ID_KERNEL_USED:%d  ", i);
        this->hopKnl[i]->p_buff_agg->ShowInfo_buff();
    }
}
template <class T>
void PartitionHop<T>::ShowInfo_buff_pp(int pp) {
    printf("\n");
    printf("Report of PingPang[%d] buff: \n", pp);
    for (int i = 0; i < this->num_knl_used; i++) {
        printf("ID_KERNEL_USED:%d  ", i);
        this->hopKnl[i]->p_buff_pp[pp]->ShowInfo_buff();
    }
}
template <class T>
void PartitionHop<T>::ShowInfo_buffs(int rnd) {
    printf("\n");
    printf("Report of status of buffs of round(%d): \n", rnd);
    for (int i = 0; i < this->num_knl_used; i++) {
        printf("ID_KERNEL_USED:%d  ", i);
        this->hopKnl[i]->p_buff_in->ShowInfo_buff("in", false);
        this->hopKnl[i]->p_buff_ping->ShowInfo_buff("ping", false);
        this->hopKnl[i]->p_buff_pang->ShowInfo_buff("pang", false);
        this->hopKnl[i]->p_buff_out->ShowInfo_buff("out", false);
        this->hopKnl[i]->p_buff_agg->ShowInfo_buff("agg", true);
    }
}
template <class T>
int PartitionHop<T>::DoSwitching(int id_src) {
    PackBuff<T>* pbf = this->hopKnl[id_src]->p_buff_out;
    long num = pbf->GetNum();
    for (int i = 0; i < num; i++) {
        HopPack<T> hpk;
        pbf->pop(&hpk);
        int idk = SelectParID(hpk.idx, this->num_knl_par, this->tab_disp_knl, this->tab_copy_knl, this->tab_state_knl);
        if (0 == this->hopKnl[idk]->p_buff_in->push(&hpk)) {
            printf("ERROR Switching faild HopKnl(%d) is full\n", i);
            return -1;
        }
    }
    return 0;
}
template <class T>
int PartitionHop<T>::DoSwitching() {
    for (int i = 0; i < num_knl_used; i++) {
        HopKernel<T>* pk = this->hopKnl[i];
        if (pk->p_buff_out->GetNum() == 0)
            continue;
        else if (DoSwitching(i) == -1)
            return -1;
    }
    return 0;
}
template <class T>
int PartitionHop<T>::DoAggregation(int id_src) {
    PackBuff<T>* pbf = this->hopKnl[id_src]->p_buff_agg;
    long num = pbf->GetNum();
    for (int i = 0; i < num; i++) {
        HopPack<T> hpk;
        pbf->pop(&hpk);
    }
    return 0;
}
template <class T>
int PartitionHop<T>::DoAggregation() {
    for (int i = 0; i < num_knl_used; i++) {
        HopKernel<T>* pk = this->HopKnl[i];
        if (pk->p_buff_agg->GetNum() == 0)
            continue;
        else if (DoAggregation(i) == -1)
            return -1;
    }
    return 0;
}

template<class T>
void PartitionHop<T>::PrintRpt(
    int numHop, int numPair, commendInfo commendInfo, timeInfo timeInfo, IndexStatistic stt
){
    T nv = this->hop_src->NV;
    T ne = this->hop_src->NE;
    double degree = (double)ne/nv;
    double allAccsEdge = 0;// the edge is accessed
    for(int i=0; i<numHop; i++){
        allAccsEdge += numPair*std::pow(degree, i+1);
        printf("INFO : allAccessEdge = %lf\n", allAccsEdge);
    }
    printf("\n");
    printf("************************************************************************************************\n");
    printf("**************************************  nHop Summary   *****************************************\n");
    printf("************************************************************************************************\n");
    printf("INFO : disturbute nHop compute time all   : %lf\n", timeInfo.timeWrkCompute);
    printf("INFO : disturbute nHop compute kernel time: %lf\n", timeInfo.timeKernel);
    printf("INFO : disturbute nHop compute memcy time : %lf\n", timeInfo.timeWrkCompute - timeInfo.timeKernel);
    printf("INFO : number Pair per second             : %lf\n", (double)numPair/timeInfo.timeKernel/1000000);//numPair/1000000/timeInfo.timeKernel*ne/nv
    printf("INFO : Estimated MTEPS for nhop           : %lf\n", (double)allAccsEdge/timeInfo.timeKernel/1000000);//allPair/1000000/timeInfo.timeKernel*ne/nv
    printf("************************************************************************************************\n");
    printf("*********************************************************\n");
    printf("************* Hardware resources for hopping ************\n");
    printf("*********************************************************\n");
    printf("Number of kernels                         : %9d\n", commendInfo.numKernel);
    printf("Number of channel in kernel               : %9d\n", commendInfo.numPuPerKernel);
    printf("Number of total channel in kernel         : %9d\n", commendInfo.numKernel * commendInfo.numPuPerKernel);
    printf("Size limitation for storing offsets(Byte) : %9d\n", limit_v_b);
    printf("Size limitation for storing indices(Byte) : %9d\n", limit_e_b);
    printf("*********************************************************\n");
    printf("************* The graph features for hopping ************\n");
    printf("*********************************************************\n");
    printf("Total number of vertex in graph           : %9d\n", nv);
    printf("Total number of edge in graph             : %9d\n", ne);
    printf("Bytes for storing graph data              : %9d\n", sizeof(T));
    printf("Total number of bytes for storing offsets : %9d\n", (nv + 1) * sizeof(T));
    printf("Total number of bytes for storing indices : %9d\n", ne * sizeof(T));
    printf("Degree of graph                           : %9.1lf\n", degree);
    printf("*********************************************************\n");
    printf("************* Commend line and input ********************\n");
    printf("*********************************************************\n");
    printf("Number of hop                             : %9d\n", numHop);
    printf("Number of kernels                         : %9d\n", commendInfo.numKernel);
    printf("Number of channel in kernel               : %9d\n", commendInfo.numPuPerKernel);
    printf("Batch size in kernel                      : %9d\n", commendInfo.sz_bat);
    printf("Duplicate graph Mode                      : %9d\n", commendInfo.duplicate);
    printf("Number of input pair                      : %9d\n", numPair);
    printf("************************************************************************************************\n");
    printf("***************************** Hopping aggregation result per kernel ****************************\n");
    printf("************************************************************************************************\n");
    for(int i = 0; i < commendInfo.numKernel; i++){
    if(commendInfo.byPass)
    printf("kernel[%d] not aggregation result          : %9d\n", i, this->hopKnl[i]->p_buff_agg->GetNum());
    else
    printf("kernel[%d] aggregation result              : %9d\n", i, this->hopKnl[i]->p_buff_agg->GetNum());
    printf("hop result for each kernel had saved      :  %s\n", this->hopKnl[i]->filename.c_str());
    }
    //printf("hop result file had saved                 :  %s\n", commendInfo.filename.c_str());
    printf("****************************************************************************************************\n");
    // printf("******* To find proper number of channel covering the entire graph and supporting intra-copy********\n");
    // printf("****************************************************************************************************\n");
}

template <class T>
int PartitionHop<T>::CreatePartitionForKernel( // Created
    int num_knl_in,
    int num_chnl_knl_in,
    T limit_nv_byte,
    T limit_ne_byte) {
    this->num_knl_used = num_knl_in;// now is the number of board // while num_knl_par is the actually number of kernel to cover the graph
    this->num_chnl_knl = num_chnl_knl_in;
    this->limit_v_b = limit_nv_byte;
    this->limit_e_b = limit_ne_byte;
    T nv = this->hop_src->NV;
    T ne = this->hop_src->NE;

    printf("\n");
    printf("*********************************************************\n");
    printf("************* Hardware resources for hopping ************\n");
    printf("*********************************************************\n");
    printf("Number of kernels                         : %9d\n", num_knl_in);
    printf("Number of channel in kernel               : %9d\n", num_chnl_knl);
    printf("Number of total channel in kernel         : %9d\n", num_knl_in * num_chnl_knl);
    printf("Size limitation for storing offsets(Byte) : %9d\n", limit_v_b);
    printf("Size limitation for storing indices(Byte) : %9d\n", limit_e_b);
    printf("*********************************************************\n");
    printf("************* The graph features for hopping ************\n");
    printf("*********************************************************\n");
    printf("Total number of vertex in graph           : %9d\n", nv);
    printf("Total number of edge in graph             : %9d\n", ne);
    printf("Bytes for storing graph data              : %9d\n", sizeof(T));
    printf("Total number of bytes for storing offsets : %9d\n", (nv + 1) * sizeof(T));
    printf("Total number of bytes for storing indices : %9d\n", ne * sizeof(T));
    printf("****************************************************************************************************\n");
    printf("******* To find proper number of channel covering the entire graph and supporting intra-copy********\n");
    printf("****************************************************************************************************\n");
    int num_ch_cover_nv = (nv * sizeof(T) + limit_v_b - 1) / (limit_v_b);
    int num_ch_cover_ne = (ne * sizeof(T) + limit_e_b - 1) / (limit_e_b);
    int num_ch_cover = num_ch_cover_nv > num_ch_cover_ne ? num_ch_cover_nv : num_ch_cover_ne;
    if (num_ch_cover >= this->num_chnl_knl) { // Size of graph > capacity of kernel
        num_knl_par = (num_ch_cover + this->num_chnl_knl - 1) / this->num_chnl_knl;
        printf("Number of kernel to cover graph (Estimated)    : %5d\n", num_knl_par);
        printf("Number of channel to cover graph (Estimated)   : %5d\n", num_knl_par * num_chnl_knl);
        this->num_chnl_par =
            CSRPartition_average(num_knl_par * num_chnl_knl, *hop_src, limit_v_b, limit_e_b, par_chnl_csr);
        while (num_chnl_par > num_knl_par * num_chnl_knl) {
            num_knl_par++;
            this->num_chnl_par =
                CSRPartition_average(num_knl_par * num_chnl_knl, *hop_src, limit_v_b, limit_e_b, par_chnl_csr);
        }
        printf("Number of minimal kernel to cover graph (Final): %5d\n", num_knl_par);
        printf("Number of channel to cover graph (Final)       : %5d\n", num_chnl_par);
    } else { // The graph tends to be stored in one kernel
        while ((0 != this->num_chnl_knl % num_ch_cover))
            num_ch_cover++; // To find proper number of channel of divid factor of num_chnl_knl
        num_knl_par = (num_ch_cover + this->num_chnl_knl - 1) / this->num_chnl_knl;
        printf("Number of kernel to cover graph (Estimated) : %5d\n", num_knl_par);
        assert(num_knl_par == 1);//const num_chnl_par=4 for kernel now
        this->num_chnl_par = CSRPartition_average_onOnekernel(/*1 * num_ch_cover*/num_chnl_knl, *hop_src, limit_v_b, limit_e_b, par_chnl_csr);/*1 * num_ch_cover*/ //todo
        while (num_chnl_par > num_knl_par * num_chnl_knl) {
            this->FreePar();
            while ((0 != this->num_chnl_knl % num_ch_cover))
                num_ch_cover++; // To find proper number of channel of divid factor of num_chnl_knl
            this->num_chnl_par = CSRPartition_average_onOnekernel(1 * num_ch_cover, *hop_src, limit_v_b, limit_e_b, par_chnl_csr);
        }
        printf("Number of channel to cover graph (Final)    : %5d\n", num_chnl_par);
        printf("Number of kernel to cover graph (Final)     : %5d\n", num_knl_par);
    }

    printf("\n");
    syncDispTab_inter();
    printf("**************************************************************************\n");
    printf("****** Deploy based on Final minimal number kernel for covering graph ****\n");
    printf("**************************************************************************\n");
    for (int i = 0; i < this->num_knl_used; i++) {
        int id_knl = i % this->num_knl_par;
        int id_ch_start = id_knl * this->num_chnl_knl;
        this->hopKnl[i] = new (HopKernel<T>);
        this->hopKnl[i]->InitCore(par_chnl_csr, id_ch_start, this->num_chnl_knl, this->num_chnl_par, this->num_knl_par,
                                  i, this->tab_disp_knl, this->tab_copy_knl);
    }
    printf("\n");
    ShowChnlPar();
    printf("\n");
    ShowDevPar();
    for (int ik_used = 0; ik_used < this->num_knl_used; ik_used++) this->hopKnl[ik_used]->ShowDispTab_intra(ik_used);
    return 0;
}

template <class T>
int PartitionHop<T>::LoadPair2Buffs(ap_uint<64>* pairs, int num_pair, T NV, T NE, int numHop, commendInfo commendInfo, timeInfo* p_timeInfo, IndexStatistic* p_stt) {
    // 1) initialize each kernels' buffs according to the size of pair, NV ,NE and numHop
    printf("\n");
    printf("Initialize each kernels' buffs according to the size of pair, NV ,NE and numHop\n");
    long sz_in = 268435456;
    long sz_pp = 268435456;
    // long sz_in = this->limit_e_b / sizeof(T);//todo
    // long sz_pp = this->limit_e_b / sizeof(T);
    long sz_out = sz_pp;
    long sz_agg = sz_pp;
    for (int i = 0; i < this->num_knl_used; i++) this->hopKnl[i]->InitBuffs(sz_in, sz_out, sz_pp, sz_agg);
    printf("Size for intput     : %9d * %d = %9d BYTEs\n", sz_in, sizeof(T), sz_in * sizeof(T));
    printf("Size for PingPang   : %9d * %d = %9d BYTEs\n", sz_pp, sizeof(T), sz_pp * sizeof(T));
    printf("Size for output     : %9d * %d = %9d BYTEs\n", sz_out, sizeof(T), sz_out * sizeof(T));
    printf("Size for aggregation: %9d * %d = %9d BYTEs\n", sz_agg, sizeof(T), sz_agg * sizeof(T));
    printf("num_knl_used : %d\n", num_knl_used);
    // 2) dispatching pairs into kernels' buff_in
    for (int i = 0; i < num_pair; i++) {
        HopPack<T> hpk;
        hpk.src = (pairs[i])(31, 0);
        hpk.des = (pairs[i])(63, 32);
        hpk.idx = hpk.src;//fixed a bug
        hpk.hop = 0;
        // int idk = FindPar<T>(hpk.idx, this->num_knl_par, this->tab_disp_knl);
        int idk = SelectParID(hpk.idx, this->num_knl_par, this->tab_disp_knl, this->tab_copy_knl, this->tab_state_knl);
        this->hopKnl[idk]->p_buff_in->push(&hpk);
        //hpk.print(i, idk);
    }
    // printf("=========\n");
    // this->hopKnl[1]->p_buff_in->print();
    // printf("=========\n");
    // ShowInfo_buff_in();

    // do 3), 4) and 5) until all are buffs empty.
    // 3) Enable batch processing for each kernel: a)first check buff_in empty? b) then do batch hopping ; exception:
    // buff full
    // 4) SW switching: a)check each buff_out and pop the package then push into proper kernel's buff_in;
    // 5) aggregation: pop package and send it into aggregator
    long sz_bat = 32768; // GetSuggestedBatch(sz_pp, numHop, NE/NV);
    long rnd = 0;
    bool allEmpty = true;
    do {
        if (rnd == 0)
            printf("\n========BEFORE HOPPING===========\n");
        else
            printf("\n========AFTER SWITCHING===========\n");
        ShowInfo_buffs(rnd);
        printf("\n========DOING  HOPPING===========\n");
        // 3) do hopping
        allEmpty = true;
        for (int i = 0; i < this->num_knl_used; i++) {
            if (this->hopKnl[i]->p_buff_in->isEmpty())
                continue;
            else if (this->hopKnl[i]->ConsumeTask(NV, NE, rnd, num_pair, sz_bat, numHop, commendInfo, p_timeInfo, p_stt) != 0) {
                return -1;
            }
            allEmpty = false;
        }
        // 4 do switching
        printf("\n========AFTER HOPPING===========\n");
        ShowInfo_buffs(rnd);
        printf("\n========DOING SWITCHING===========\n");
        this->DoSwitching();
        // 5 do aggregation
        rnd++;
    } while (allEmpty == false);
    /*ShowInfo_buff_out();
    ShowInfo_buff_agg();
    ShowInfo_buff_pp(0);
    ShowInfo_buff_pp(1);*/
    ShowInfo_buffs(rnd);
    for(int i = 0; i < this->num_knl_used; i++){
        this->hopKnl[i]->filename = commendInfo.filename + "_" + to_string(i);
        if(commendInfo.output)
            this->hopKnl[i]->p_buff_agg->SavePackBuff(this->hopKnl[i]->p_buff_agg->GetNum(), this->hopKnl[i]->filename.c_str());
    }
    return 0;
}

template <class T>
int PartitionHop<T>::MergeResult(commendInfo commendInfo){
    typedef std::pair<long,long> Key_pair;
    std::map<Key_pair, int> result;
    std::map<Key_pair, int>::iterator itr;
    for(int i = 0; i < this->num_knl_used; i++){
        int num = this->hopKnl[i]->p_buff_agg->GetNum();
        for(int j = 0; j < num; j++){
            T src = this->hopKnl[i]->p_buff_agg[j]->pBuff->src;
            T des = this->hopKnl[i]->p_buff_agg[j]->pBuff->des;
            T agg = this->hopKnl[i]->p_buff_agg[j]->pBuff->idx;
            Key_pair Keypair(src, des);
            itr = result.find(Keypair);
            if (itr != result.end()){
                result.at(Keypair) += agg;
            }else{
                result.insert(std::pair<Key_pair, int>(Keypair, agg));
            }
        }
    }

    int num = result.size();
    FILE* fp=fopen(commendInfo.filename.c_str(), "w");
    fprintf(fp,"%ld\n", num);
    for(itr = result.begin(); itr != result.end(); ++itr){
        Key_pair Keypair= itr->first;
        fprintf(fp, "%ld %ld %ld \n", Keypair.first, Keypair.second, itr->second);
    }
    fclose(fp);
}

template <class T>
long HopKernel<T>::estimateBatchSize(int cnt_hop, long sz_suggest, PackBuff<T>* p_buff_pop) {
    long sz_bat = sz_suggest;
    if (cnt_hop == 0)
        sz_bat = this->p_buff_in->GetNum() > sz_bat ? sz_bat : p_buff_in->GetNum();
    else
        sz_bat = p_buff_pop->GetNum(); // currently we hop the entire batch's middle package can be processed
    // But in future, there should be some method to avoid overflow
    return sz_bat;
}

template <class T>
int HopKernel<T>::ConsumeTask(T NV, T NE, int rnd, T numSubpair, long sz_bat, int numHop,
     commendInfo commendInfo, timeInfo* p_timeInfo, IndexStatistic* p_stt
) {
    int cnt_hop = 0;
    long num_idxs = 0;
    // IndexStatistic* p_stts[MAX_NUM_HOP];
    printf("INFO: ID_KERNEL_USED=%d, in batch-round %d to consume up to %d packages with %d hop(s)\n", this->id_knl_used, rnd,
           commendInfo.sz_bat, numHop);
    do {
        // p_stts[cnt_hop] = new(IndexStatistic);
        // IndexStatistic stt;
        PackBuff<T>* p_buff_pop = (cnt_hop == 0) ? this->p_buff_in : this->p_buff_pp[cnt_hop & 1];
        PackBuff<T>* p_buff_send = this->p_buff_out;
        PackBuff<T>* p_buff_local = this->p_buff_pp[(cnt_hop + 1) & 1];
        PackBuff<T>* p_buff_agg = this->p_buff_agg;
        long sz_bat2 = estimateBatchSize(cnt_hop, sz_bat, p_buff_pop);//TODO
        long size_pop = p_buff_pop->getSize();
        printf("%s : cnt_hop=%d, sz_bat=%ld, sz_bat2=%ld, numHop=%d, size_in_pop=%d, num_pair=%d\n", __FUNCTION__, cnt_hop, sz_bat,
               sz_bat2, numHop, size_pop, numSubpair);
        // if( -1==BatchOneHop(p_buff_pop, p_buff_send, p_buff_local, sz_bat2, numHop, &stt)){
        if (-1 ==
            BatchOneHopOnFPGA(p_buff_pop, p_buff_send, p_buff_local, p_buff_agg, 
            NV, NE, numSubpair, numHop, sz_bat2, commendInfo, p_timeInfo, p_stt)) {
            printf("Error: batch size(%ld) can't be consumed out, created %ld at hop(%d)\n", sz_bat2, p_stt->num_all,
                   cnt_hop);
            printf("Please try small size of batch\n");
            return -1;
        }
        cnt_hop+= numHop;
        //cnt_hop++;
        printf("  [Hop(%d):", cnt_hop);
        p_stt->print();
        printf("]\n");
        printf("Judge: cnt_hop=%d, numHop=%d\n",cnt_hop, numHop);
    } while (cnt_hop < numHop);
    return 0;
}

// hop processing 1 batch of vertex
template <class T>
int HopKernel<T>::BatchOneHop(PackBuff<T>* p_buff_pop,
                              PackBuff<T>* p_buff_send,
                              PackBuff<T>* p_buff_local,
                              long sz_bat,
                              int numHop,
                              IndexStatistic* p_stt) {
    // PackBuff<T>* p_buff_pp_w = this->p_buff_pp[cnt_bat&1];
    long sz_idx = 0;
    long sz_pop = 0;
    for (int i = 0; i < sz_bat; i++) {
        HopPack<T> in;
        p_buff_pop->pop(&in);
        AccessPoint<T> accp;
        this->LookUp(&accp, &in);
        p_stt->num_v++;
        p_stt->num_all += accp.degree;
        for (int d = 0; d < accp.degree; d++) {
            HopPack<T> out;
            out.src = in.src;
            out.des = in.des;
            out.idx = accp.p_idx[d];
            out.hop = in.hop + 1;
            // judge out 1)agg;
            if (out.des == out.idx) { // Judge_equal<T>(out)
                p_stt->num_agg++;
                if (0 == this->p_buff_agg->push(&out)) return -1;
                p_stt->num_push++;
            }
            // hop done
            if (out.hop == numHop) {
                p_stt->num_free++;
                continue;
            } else { // considering stay locally or send to others
                int idk = SelectParID<T>(out.idx, this->num_knl_par, this->tab_disp_knl, this->tab_copy_knl,
                                         this->tab_state_knl);
                if (idk % this->num_knl_par == GetKnlParID()) {
                    p_stt->num_local++;
                    if (0 == p_buff_local->push(&out)) return -1;
                    p_stt->num_push++;
                } else {
                    p_stt->num_send++;
                    if (0 == p_buff_send->push(&out)) return -1;
                    p_stt->num_push++;
                }
            }
        }
    }
    return 0;
}

// hop processing 1 batch of vertex on FPGA
template <class T>
int HopKernel<T>::BatchOneHopOnFPGA(PackBuff<T>* p_buff_pop,
                                    PackBuff<T>* p_buff_send,
                                    PackBuff<T>* p_buff_local,
                                    PackBuff<T>* p_buff_agg,
                                    T NV,
                                    T NE,
                                    T numSubPairs,
                                    int numHop,
                                    T estimateBatchSize,
                                    commendInfo commendInfo,
                                    timeInfo* p_timeInfo,
                                    IndexStatistic* p_stt) {
    // PackBuff<T>* p_buff_pp_w = this->p_buff_pp[cnt_bat&1];
    T sz_idx = 0;
    T sz_pop = 0;
    int num_chnl = commendInfo.numPuPerKernel;
    int pu = num_chnl;
    T numVertices = NV;
    T numEdges = NE;
    T numPairs = p_buff_pop->GetNum();//numSubPairs;//rename the numSubPairs for the host work
    T batchSize = commendInfo.sz_bat;//todo estimateBatchSize;//commendInfo.sz_bat;
    int byPass = commendInfo.byPass;
    int duplicate = commendInfo.duplicate;
    std::string xclbin_path = commendInfo.xclbin_path;
    printf("INFO: final batch size : %d, numPairs=%ld, numSubPairs=%ld\n", batchSize, numPairs, numSubPairs);

    // dispatch offset and index
    unsigned* offsetTable;
    offsetTable = aligned_alloc<unsigned>(1024);
    unsigned* indexTable;
    indexTable = aligned_alloc<unsigned>(1024);
    ap_uint<64>* cardTable;
    cardTable = aligned_alloc<ap_uint<64> >(1024);

    // if (duplicate == 1) {
    //     for (int i = 0; i < num_chnl + 1; i++) {
    //         offsetTable[i] = NV; // channels
    //         indexTable[i] = NE;
    //     }
    //     offsetTable[num_chnl + 1] = 0;
    //     offsetTable[num_chnl + 2] = NV;
    // }

    T* offsetBuffer[num_chnl];
    T* indexBuffer[num_chnl];
    if (duplicate == 0) {// split the subgraph to n channels avg
        for (int i = 0; i < pu; i++) {
            HopChnl<T>* pch = &(channels[i]);
            if(i == 0){
                offsetTable[0] = pch->pCSR->V_start;//offsetShift
                indexTable[0] = pch->pCSR->offset[offsetTable[0]];//indexShift
            }
            offsetTable[i+1] = pch->pCSR->V_end_1;
            indexTable[i+1] = pch->pCSR->E_end;//indexShift;//
            offsetBuffer[i] = aligned_alloc<unsigned>(pch->pCSR->NV + 1 + 4096);
            indexBuffer[i] = aligned_alloc<unsigned>(pch->pCSR->NE + 4096);
            memcpy((void*)(offsetBuffer[i]), (void*)pch->pCSR->offset,  (pch->pCSR->NV+1)*sizeof(T));
            memcpy((void*)(indexBuffer[i]), (void*)pch->pCSR->index,  (pch->pCSR->NE)*sizeof(T));
        }
        for (int i = 0; i < pu+1; i++) {
            std::cout << "INFO: id=" << i << " offsetTable=" << offsetTable[i] << " indexTable=" << indexTable[i]
                      << std::endl;
        }
    } else { // duplicate the subgraph to n channels, because the graph is small < 1 channel capability
        offsetTable[pu] = numVertices;
        for (int i = 0; i < pu; i++) {
            offsetTable[i] = numVertices;
            offsetBuffer[i] = aligned_alloc<unsigned>(numVertices + 4096);
        }
        for (int i = 0; i < pu; i++) {
            for (int j = 0; j < numVertices + 1; j++) {
                offsetBuffer[i][j] = channels->pCSR->offset[j];
            }
        }

        for (int i = 0; i < pu+1; i++) {
            indexTable[i] = channels->pCSR->offset[offsetTable[i]];
        }
        for (int i = 0; i < pu+1; i++) {
            std::cout << "id=" << i << " offsetTable=" << offsetTable[i] << " indexTable=" << indexTable[i]
                      << std::endl;
        }
        for (int i = 0; i < pu; i++) {
            indexBuffer[i] = aligned_alloc<unsigned>(numEdges + 4096);
            for (int j = 0; j < numEdges; j++) {
                indexBuffer[i][j] = channels->pCSR->index[j];
                // std::cout << "i=" << i << " j=" << j - indexTable[i] << " index=" << channels->pCSR->index[j] << std::endl;
            }
        }
    }
    offsetTable[pu+1] = offsetTable[0];
    offsetTable[pu+2] = offsetTable[pu];

    for (int i = 0; i < 32; i++) {
        ap_uint<32> id = i;
        ap_uint<32> tmp = NV * i;
        cardTable[i] = (tmp, id);
    }

    // pair
    ap_uint<128>* pair = aligned_alloc<ap_uint<128> >(numPairs + 4096);
    for (int i = 0; i < numPairs; i++) {
        HopPack<T> in;
        p_buff_pop->pop(&in);
        ap_uint<128> tmp128;
        tmp128.range(31, 0) = in.src;
        tmp128.range(63, 32) = in.des;
        tmp128.range(95, 64) = in.idx;
        tmp128.range(127, 96) = in.hop; // hop_cnt
        pair[i] = tmp128;
        //if(in.src == in.des)
        //printf("selfloop## src: %d, dst: %d\n", in.src, in.des);
    }

    // initilaize buffer and config
    unsigned* numOut = aligned_alloc<unsigned>(1024);
    ap_uint<512>* local = aligned_alloc<ap_uint<512> >(3 << 20);
    ap_uint<512>* netSwitch = aligned_alloc<ap_uint<512> >(3 << 20);
    ap_uint<512>* zeroBuffer0 = aligned_alloc<ap_uint<512> >(4 << 20);
    ap_uint<512>* zeroBuffer1 = aligned_alloc<ap_uint<512> >(4 << 20);
    for(int i=0; i<(4 << 20); i++ ){
        if(i<(3 << 20)){
            local[i] = 0;
            netSwitch[i] = 0;
        }
        zeroBuffer0[i] = 0;
        zeroBuffer1[i] = 0;
    }
    numOut[0] = 0;
    numOut[1] = 0;

    // do pre-process on CPU
    //struct timeval start_time, end_time;

#ifndef HLS_TEST

    nHop_L2_host(numHop, 
                    0,
                    numPairs,  
                    batchSize, //--batch 4096
                    65536,
                    byPass,    
                    duplicate, 
                    (ap_uint<512>*)pair,
                    offsetTable, indexTable, cardTable, offsetBuffer, (ap_uint<128>**)indexBuffer,
                    zeroBuffer0, zeroBuffer1, numOut, local, netSwitch,
                    numVertices, numEdges, pu, p_timeInfo, xclbin_path
                    );

#else

    nHop_kernel(numHop, 
                0,
                numPairs,  
                batchSize, //--batch 4096
                65536,
                byPass,    
                duplicate,
                (ap_uint<512>*)pair,
                offsetTable, indexTable, cardTable, offsetBuffer[0], (ap_uint<128>*)indexBuffer[0], offsetBuffer[1], (ap_uint<128>*)indexBuffer[1],
                offsetBuffer[2], (ap_uint<128>*)indexBuffer[2], offsetBuffer[3], (ap_uint<128>*)indexBuffer[3], 
    #if (pu==8)             
                offsetBuffer[4], (ap_uint<128>*)indexBuffer[4],
                offsetBuffer[5], (ap_uint<128>*)indexBuffer[5], 
                offsetBuffer[6], (ap_uint<128>*)indexBuffer[6], 
                offsetBuffer[7], (ap_uint<128>*)indexBuffer[7],
    #endif
                zeroBuffer0, zeroBuffer1, numOut, local, netSwitch);
#endif


    std::cout << "numLocal=" << numOut[0] << " numSwitch=" << numOut[1] << std::endl;
    std::map<unsigned long, float> sortMap;
    for (int i = 0; i < numOut[0]; i++) {
        for (int j = 0; j < 4; j++) {
            unsigned long tmp_src = local[i].range(128 * j + 31, 128 * j);
            unsigned long tmp_des = local[i].range(128 * j + 63, 128 * j + 32);
            unsigned long tmp_res = local[i].range(128 * j + 95, 128 * j + 64);
            unsigned long tmp = 0UL | (tmp_src + 1) << 32UL | (tmp_des + 1);
            if ((tmp_src != 0) && (tmp_des != 0)) sortMap.insert(std::pair<unsigned long, unsigned>(tmp, tmp_res));
        }
    }
#ifdef printresult
    for(auto it=sortMap.begin(); it!=sortMap.end(); it++) {
        unsigned long tmp_src = (it->first) / (1UL << 32UL);
        unsigned long tmp_des = (it->first) % (1UL << 32UL);
        unsigned long tmp_res = it->second;
        std::cout << "kernel result: src=" << tmp_src << " des=" << tmp_des << " res=" << tmp_res << std::endl;
    }
#endif
    std::cout << "kernel done!" << std::endl;
    std::cout << "numLocal=" << numOut[0] << " numSwitch=" << numOut[1] << std::endl;

    for (int i = 0; i < numOut[0]; i++) {
        for (int j = 0; j < 4; j++) {
            //unsigned long tmp = 0UL | (tmp_src + 1) << 32UL | (tmp_des + 1);
            HopPack<T> out;
            out.src = local[i].range(128 * j + 31, 128 * j);
            out.des = local[i].range(128 * j + 63, 128 * j + 32);
            out.idx = local[i].range(128 * j + 95, 128 * j + 64);
            out.hop = local[i].range(128 * j + 127, 128 * j + 96);

            if ((out.src != 0) && (out.des != 0)){
                p_stt->num_agg++;
                //if (0 == p_buff_local->push(&out)) return -1;
                if (0 == p_buff_agg->push(&out)) return -1;
                p_stt->num_push++;
            }
        }
    }
    std::cout << " aggnum=" << p_buff_agg->GetNum() << std::endl;

    std::cout << " numSwitch=" << numOut[1] << std::endl;
    for (int i = 0; i < numOut[1]; i++) {
        for (int j = 0; j < 4; j++) {
            HopPack<T> out;
            out.src = netSwitch[i].range(128 * j + 31, 128 * j);
            out.des = netSwitch[i].range(128 * j + 63, 128 * j + 32);
            out.idx = netSwitch[i].range(128 * j + 95, 128 * j + 64);
            out.hop = netSwitch[i].range(128 * j + 127, 128 * j + 96);

            if ((out.src != 0) && (out.des != 0)){
                p_stt->num_send++;
                if (0 == p_buff_send->push(&out)) return -1;
                p_stt->num_push++;
            }
        }
    }

    for(int i = 0; i < pu; i++){
        free(offsetBuffer[i]);
        free(indexBuffer[i]);
    }

    return 0;
}

// int HoppingSwitching(ap_uint<64>* pairs, int num_pair, long NV, long NE, int numHop){
//     // do 3), 4) and 5) until all are buffs empty.
//     //3) Enable batch processing for each kernel: a)first check buff_in empty? b) then do batch hopping ; exception:
//     buff full
//     //4) SW switching: a)check each buff_out and pop the package then push into proper kernel's buff_in;
//     //5) aggregation: pop package and send it into aggregator
//     long sz_bat = 100;// GetSuggestedBatch(sz_pp, numHop, NE/NV);
//     long rnd=0;
//     bool allEmpty=true;
//     do{
//         if(rnd==0)
//             printf("\n========BEFORE HOPPING===========\n");
//         else
//             printf("\n========AFTER SWITCHING===========\n");
//         ShowInfo_buffs(rnd);
//         printf("\n========DOING  HOPPING===========\n");
//         //3) do hopping
//         allEmpty=true;
//         for(int i=0; i<this->num_knl_used;  i++){
//             if(this->hopKnl[i]->p_buff_in->isEmpty())
//                 continue;
//             else if(this->hopKnl[i]->ConsumeTask( rnd, sz_bat,  numHop)!=0){
//                 return -1;
//             }
//             allEmpty =false;
//         }
//         //4 do switching
//         printf("\n========AFTER HOPPING===========\n");
//         ShowInfo_buffs(rnd);
//         printf("\n========DOING SWITCHING===========\n");
//         this->DoSwitching();
//         //5 do aggregation
//         rnd++;
//     }while(allEmpty==false);
//     /*ShowInfo_buff_out();
//     ShowInfo_buff_agg();
//     ShowInfo_buff_pp(0);
//     ShowInfo_buff_pp(1);*/
//     ShowInfo_buffs(rnd);
//     return 0;
// }
#endif
