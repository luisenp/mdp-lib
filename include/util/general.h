struct pair_int_hash {
    inline std::size_t operator()(const std::pair<int,int> & v) const {
        return v.first*31+v.second;
    }
};

typedef std::unordered_set< std::pair <int,int> , pair_int_hash> IntPairSet;
