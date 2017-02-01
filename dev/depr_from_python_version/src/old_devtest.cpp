#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#include <tinygemm/tinygemmerror.hpp>
#include <tinygemm/tinygemm.hpp>
#include <tinygemm/devtinygemm.hpp>
#include <tinygemm/kernelstringgenerator.hpp>
#include <tinygemm/stringutilbase.hpp>
#include <tinygemm/hyperparams.hpp>
#include <tinygemm/tinygemmgeometry.hpp>

/* Note that currently (13/11/2016) most testing is done through dev/python scripts */



class DevGemmTester{
  
  public:

    /* ** The parameters common for testing gemmbench and find ********* */
    bool isColMajor;
    bool tA;
    bool tB;
    bool tC;
    size_t m;    
    size_t n;
    size_t k;
    float alpha;
    float beta;
    size_t lda;    
    size_t ldb;
    size_t ldc;
    size_t a_offset;    
    size_t b_offset;
    size_t c_offset;
    
    
    std::vector<float> v_c;
    std::vector<float> v_a;
    std::vector<float> v_b;
    std::string outputfilename;
    bool capture_output;
    std::string output;
    bool do_test;
    size_t n_runs;
    /* ****************************************************************** */
    
    DevGemmTester(){
      /* free to change these parameter values */
      /* ************************************* */
      isColMajor = false;
      tA = true;
      tB = false;
      tC = false;
      m = 4532;    
      n = 3322;
      k = 3131;
      lda = tA == isColMajor ? k + 1 : m + 6;    
      ldb = tB == isColMajor ? n + 2 : k + 5;
      ldc = tC == isColMajor ? n + 3 : m + 4;
      a_offset = 1;
      b_offset = 2;
      c_offset = 3;
      alpha = 1.1;
      beta = 1.1;
      outputfilename = "";    
      capture_output = false;
      output = "";
      n_runs = 10;
      /* ************************************ */
      
      
      
      
      size_t n_a = lda * (tA == isColMajor ? m : k);
      size_t n_b = ldb * (tB == isColMajor ? k : n);
      size_t n_c = ldc * (tC == isColMajor ? m : n); 
      
      /* TODO get the true post-gemm C so that we can do a test 
       * (or just keep the main testing from dev/python)
       * update : consider geometry tests */
      do_test = false;
      
      /* fill matrices with random floats. 
       * Note : if they're integers, the kernel runs faster! */
      v_a.resize(n_a); 
      for (size_t i = 0; i < n_a; ++i){
        v_a[i] = float(rand() % 1000) / 1000. - 0.5;
      }


      
      v_b.resize(n_b);
      for (size_t i = 0; i < n_b; ++i){
        v_b[i] = float(rand() % 1000) / 1000. - 0.5;
      }


      v_c.resize(n_c);
      for (size_t i = 0; i < n_c; ++i){
        v_c[i] = float(rand() % 1000) / 500 - 1.;
      }
      
    }
    
    int red_benchmark(const std::vector< tinygemm::hyperparams::HyperParams > & hps, bool findfirst, float allotted_time, bool enforce_deterministic = false){
      /* We pass cpu pointers to tinygemm::dev::benchgemm, which does all the necessary opencl gpu boilerplating */
      
      tinygemm::dev::benchgemm<float>(isColMajor, tA, tB, tC, m, n, k, alpha, v_a.data(), lda, a_offset, v_b.data(), ldb, b_offset, beta, v_c.data(), ldc, c_offset, {}, hps, capture_output, output, nullptr, do_test, n_runs, outputfilename, findfirst, allotted_time, enforce_deterministic);
      return 0;

    }
      
  
    int call_benchgemm(){
      //std::string kernel_string = get_a_kernel_string();
      //std::vector<std::vector<std::string> > gpu_kernel_strings {{ kernel_string }};
      std::cout << "Hello from tinygemm call_benchgemm test!\n";  
      red_benchmark({ get_hp() }, false, -1.0, false);
      return 0;
    }
    
    int call_find(){
      //std::vector<std::vector<std::string> > gpu_kernel_strings {};
      red_benchmark( {}, true, 30.0, false); //10 seconds, don't force to be determinisitc
      return 0;
    }


    tinygemm::hyperparams::HyperParams get_hp(){

      std::map<std::string, unsigned> hp_map;
      hp_map["micro_tile_width"] = 1;
      hp_map["micro_tile_height"] = 1;
      hp_map["macro_tile_width"] = 16;
      hp_map["macro_tile_height"] = 16; 
      hp_map["unroll"] = 16;
      
      hp_map["pad"] = 1;    
      hp_map["group_allocation"] = 1;
      hp_map["work_item_load_a_pll_to_unroll"] = 0;
      hp_map["work_item_load_b_pll_to_unroll"] = 1;
      hp_map["unroll_pragma"] = 1;
      
      hp_map["load_to_lds_interwoven"] = 0;
      hp_map["c_micro_tiles_interwoven"] = 1;
      hp_map["n_work_items_per_c_elm"] = 2;
      hp_map["unroll_for_offset"] = 0;
      hp_map["n_target_active_workgroups"] = 64;
      
      return hp_map;

    }

    /* testing the generating of a kernel string (writes to terminal)  */
    /* previous versions of tinygemm would use a python script to write the kernel to a file like so :
     * tinygemm::mkkern::make_kernel_via_python(dir_name, t_float, all_int_parms, "somekernelname", true); */
    
    
    std::string get_a_kernel_string(){
      
      
      
      std::string kernel_string;
      
      tinygemm::kerngen::KernelStringSetStatus set_status = tinygemm::kerngen::set_kernel_string(
      get_hp(),
      kernel_string,
     "somekernelname",
      32, //"float" ? 32 : 64,
      tA, 
      tB, 
      tC, 
      isColMajor
      );
      
      
      if (set_status.is_good() == false){
        throw tinygemm::tinygemm_error(set_status.message);
      }
        
      return kernel_string;
    }
};



int main (){
    
  DevGemmTester dgt;
  
  bool test_benchmark = true;
  if (test_benchmark == true){
    dgt.call_benchgemm();
  }
  
  bool test_make_kernel = false;  
  if (test_make_kernel == true){
    std::string bla = dgt.get_a_kernel_string();
    std::cout << bla << std::endl;
  }

  bool test_find = false;  
  if (test_find == true){
    dgt.call_find();  
  }
  
  return 0;
}
