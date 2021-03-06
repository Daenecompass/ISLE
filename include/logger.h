// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <fstream>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <numeric>

#include "Eigen/Core"

#include "types.h"
#include "hyperparams.h"

namespace ISLE
{

    class Logger
    {

        std::ofstream out_log;

    public:
        Logger(const std::string& filename)
        {
            try {
                out_log.open(filename);
            }
            catch (std::ios_base::failure& e) {
                std::cerr << e.what() << '\n';
                exit(-1);
            }
        }
        ~Logger()
        {
            out_log.close();
        }

        void print_string(
            const std::string& str,
            const bool print_to_terminal = true)
        {
            out_log << str << std::flush;
            if (print_to_terminal)
                std::cout << str << std::flush;
        }

        void print_stringstream(
            const std::ostringstream& stream,
            const bool print_to_terminal = true)
        {
            out_log << stream.str() << std::flush;
            if (print_to_terminal)
                std::cout << stream.str() << std::flush;
        }

        template <class catchT>
        void print_catch_words(const doc_id_t topic,
            const catchT* catch_threshold,
            const std::vector<word_id_t>& catchwords,
            const std::vector<std::string>& vocab_words,
            bool print_to_terminal = true)
        {
            std::ostringstream ostr;
            ostr << "Catchwords:" << "\n";
            for (auto iter = catchwords.begin(); iter != catchwords.end(); ++iter)
                ostr << vocab_words[*iter]
                << ":" << *iter
                << "(" << catch_threshold[*iter] << ") ";
            ostr << "\n";

            print_stringstream(ostr, print_to_terminal);
        }

        void print_cluster_details(const doc_id_t num_topics,
            const std::vector<FPTYPE>& distsq,
            const std::vector<word_id_t> *const catchwords,
            const std::vector<doc_id_t> *const closest_docs,
            const std::vector<FPTYPE>& coherences,
            const std::vector<FPTYPE>& nl_coherences,
            bool print_to_terminal = true)
        {
            std::ostringstream ostr;

            std::vector<std::pair<int, doc_id_t> > cluster_sizes;
            for (auto t = 0; t < num_topics; ++t)
                cluster_sizes.push_back(std::make_pair(closest_docs[t].size(), t));
            std::sort(cluster_sizes.begin(), cluster_sizes.end(),
                [](const auto& left, const auto&right) {return left.first < right.first; });

            int catchless = 0;
            for (auto i = 0; i < num_topics; ++i) {
                auto t = cluster_sizes[i].second;
                ostr << std::setw(12) << std::left << "Cluster" << t
                    << std::setw(12) << std::left << "  size:" << cluster_sizes[i].first
                    << std::setw(15) << std::left << "  distsq_sum:" << distsq[i]
                    << std::setw(15) << std::left << "  raw_coh:" << nl_coherences[i]
                    << std::setw(15) << std::left << "  flt_coh:" << coherences[i]
                    << "  #catchwords: " << catchwords[t].size() << std::endl;

                if (catchwords[t].size() == 0)
                    catchless++;
            }
            ostr << "\n#Topics with no catchwords: " << catchless
                << "(" << num_topics << ")" << std::endl;

            print_stringstream(ostr, print_to_terminal);
        }

        void print_eigen_data(
            Eigen::Matrix<FPTYPE, Eigen::Dynamic, 1>& evalues,
            doc_id_t num_topics,
            bool print_to_terminal = true)
        {
            std::ostringstream ostr;

            ostr << "Eigvals:  ";
            for (doc_id_t t = 0; t < num_topics; ++t)
                ostr << "(" << t << "): " << std::sqrt(evalues(t)) << "\t";
            ostr << std::endl;
            std::vector<FPTYPE> eig_sum_slabs(num_topics / 100 + 1, 0.0);
            for (doc_id_t t = 0; t < num_topics; ++t)
                eig_sum_slabs[t / 100] += evalues(t);
            for (auto slab = 0; slab < num_topics / 100; ++slab)
                ostr << "Sum of Top-" << (slab + 1) * 100 << " eig vals: "
                << std::accumulate(eig_sum_slabs.begin(), eig_sum_slabs.begin() + 1 + slab, (FPTYPE)0.0)
                << "\n";

            print_stringstream(ostr, print_to_terminal);
        }
    };
}
