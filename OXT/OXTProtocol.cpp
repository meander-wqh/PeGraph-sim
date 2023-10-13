#include "OXTProtocol.h"
#include <openssl/rand.h>
#include "../Util/Utils.h"

OXT::OXT(const char* dataset){
    file.open(dataset);
    if(!file.is_open()){
        std::cerr << "无法打开文件" << std::endl;
    }
    //读文件
	double ratio = 1;
    int lineNumber = 0;
    std::string line;
    int i=0;
    while (std::getline(file, line)) {
		if(lineNumber>=Fileline*ratio){
			break;
		}
        // 使用字符串流将每行分割成元素item
        std::istringstream iss(line);
        std::string item;
        std::vector<std::string> items;
        while (iss >> item) {
            items.push_back(item);
        }
		std::string sw = "fd"+items[0];
		std::string sid = items[1];
        if(DB.find(sw) != DB.end()){
            DB[sw].push_back(sid);
        }else{
            vector<string> ids;
            DB[sw] = ids;
        }
        lineNumber++;
    }
    // pbc_param_t param;
    // size_t count;
    // char *params = NULL;
    // 读取参数文件
    // FILE *fp = fopen("../pbcwrapper/pairing.param", "r");
    // fseek(fp, 0, SEEK_END);
    // count = ftell(fp);
    // rewind(fp);
    // params = (char *)malloc(count * sizeof(char));
    // fread(params, 1, count, fp);
    // fclose(fp);

    // // 初始化PBC库
    // pbc_param_init_set_str(param, params);
    // pbc_param_init_a_gen(param, 160, 512);
    // pairing_init_pbc_param(pairing, param);

    // // 随机生成G1群元素
    // element_init_G1(g, pairing);
    // element_random(g);

    const char *paramFileName = "pbcwrapper/pairing.param";
    FILE *sysParamFile = fopen(paramFileName, "r");
    if (sysParamFile == NULL) {
        cerr<<"Can't open the parameter file " << paramFileName << "\n";
        return;
    }
    p = new Pairing(sysParamFile);
    cout<<"Is symmetric? "<< p->isSymmetric()<< endl;
    cout<<"Is pairing present? "<< p->isPairingPresent()<< endl;  
    fclose(sysParamFile);
    
    g = G1(*p);
}

OXTEDB OXT::SetUp(){
    OXTEDB oxtedb;

    //生成密钥
    RAND_bytes(oxtedb.K_I,ENC_KEY_SIZE); 
    RAND_bytes(oxtedb.K_Z,ENC_KEY_SIZE); 
    RAND_bytes(oxtedb.K_T,ENC_KEY_SIZE); 
    RAND_bytes(oxtedb.K_S,ENC_KEY_SIZE); 
    RAND_bytes(oxtedb.K_X,ENC_KEY_SIZE);
    RAND_bytes(oxtedb.K_E,ENC_KEY_SIZE); 

    for(auto iter = DB.begin(); iter!=DB.end(); iter++) {
        string sw = iter->first;
        int c = 1;
        for(int i=0;i<iter->second.size();i++){
            string sid = iter->second[i];

            //Exact item
            const char* w = sw.c_str();
            size_t w_len = sw.length();
            const char* id = sid.c_str();
            size_t id_len = sid.length();

            //加密xind
            unsigned char xind[id_len + AESGCM_MAC_SIZE + AESGCM_IV_SIZE];
            int xind_len = enc_aes_gcm((unsigned char*)id, id_len, oxtedb.K_I, xind);

            unsigned char z[to_string(c).length() + AESGCM_MAC_SIZE + AESGCM_IV_SIZE];
            int z_len = enc_aes_gcm((unsigned char*)to_string(c).c_str(), to_string(c).length(), oxtedb.K_Z, z);

            Zr xind_Zr(*p,xind,xind_len);
            Zr z_Zr(*p,z,z_len);
            Zr y(*p,true);
            y = xind_Zr/z_Zr;
            // vector<uint8_t> vxind(xind,xind+xind_len);
            // HashResultZr xindhash(pairing,vxind); 
            // element_t resultXind;
            // resultXind[0] = xindhash.element[0];

            // vector<uint8_t> vz(z,z+z_len);
            // HashResultZr zhash(pairing,vz);
            // element_t resultZ;
            // resultZ[0] = zhash.element[0];
            
            // element_t y;
            // element_init_Zr(y,pairing);
            // element_div(y,resultXind,resultZ);

            unsigned char stag[w_len+ AESGCM_MAC_SIZE + AESGCM_IV_SIZE];
            int stag_len = enc_aes_gcm((unsigned char*)w, w_len, oxtedb.K_T, stag);

            unsigned char e[id_len + AESGCM_MAC_SIZE + AESGCM_IV_SIZE];
            int e_len = enc_aes_gcm((unsigned char*)id, id_len, oxtedb.K_E, e);

            OXTTSetTuple tuple;
            tuple.e = e;
            tuple.y = y;

            if(oxtedb.TSet.find(stag) != oxtedb.TSet.end()){
                oxtedb.TSet[stag].push_back(tuple);
            }

            unsigned char kxw[w_len+ AESGCM_MAC_SIZE + AESGCM_IV_SIZE];
            int kxw_len = enc_aes_gcm((unsigned char*)w, w_len, oxtedb.K_X, kxw);

            Zr kxw_Zr(*p,kxw,kxw_len);
            G1 xtag(*p);
            xtag = g ^ (kxw_Zr * xind_Zr);

            // vector<uint8_t> vkxw(kxw,kxw + kxw_len);
            // HashResultZr kxwhash(pairing,vkxw);
            // element_t resultkxw;
            // resultkxw[0] = kxwhash.element[0];

            // element_t xtag;
            // element_init_GT(xtag, pairing);
            // element_mul(xtag, resultkxw, resultXind);
            // element_pow_zn(xtag, g, xtag);
            oxtedb.XSet.push_back(xtag.toString(true));
            c++;
        }
    }
    cout<<"finished"<<endl;
    file.close();
    return oxtedb;
}
