#include "WaveReader.hpp"
#include "FFT.hpp"
#include <iostream>
#include <iterator>

void WaveReader::Read() {
    ReadMainChank();
    ReadSubChank1();
    ReadSubChank2();
}

void WaveReader::PrintInfo() {
    std::cout << "ChunckID: "<<header_.chunk_id_
              <<"\nChunck size: "<<header_.chunk_size_
              <<"\nFormat: "<<header_.format_
              <<"\nSubChunck1ID: "<<header_.sub_chank1_id_
              <<"\nFormat Code: "<<header_.format_code_
              <<"\nNumber of channels: "<<header_.number_of_channels_
              <<"\nSamples per second: "<<header_.samples_per_second_
              <<"\nBytes per second: "<<header_.bytes_per_second_
              <<"\nBytes per sample frame: "<<header_.bytes_per_sample_frame_
              <<"\nBits per sample: "<<header_.bits_per_sample_
              <<"\nSubChank2ID: "<<header_.sub_chank2_id_
              <<"\nSubChank2Size: "<<header_.sub_chank2_size_;
    std::cout<<"\nSamples: "<<header_.number_of_samples_<<"\n";
    std::cout<<"Samples 1 size: "<<samples_.size()<<"\n";
    std::cout<<"Samples 2 size: "<<samples2_.size()<<"\n";
    SaveSamplesToFile();
    CalculateFFT();
}

void WaveReader::ReadXBitToString(std::string& str, const size_t X) {
    char tmp[X+1] {};
    for (int i = 0; i<X; i++)
        file_.get(tmp[i]);
    str = std::string(tmp);
}

void WaveReader::ReadSamples(const int number_of_samples, const int channels) {
    int16_t sample;
    for (int i=0;i<channels*number_of_samples;i++) {
        file_.read(reinterpret_cast<char* >(&sample), sizeof(sample));

        if (channels == 2)
            (i%2==0) ? samples_.emplace_back(sample) : samples2_.emplace_back(sample);
        else if (channels == 1)
            samples_.emplace_back(sample);
    }
}

uint32_t WaveReader::CalculateSamplesNumber() {
    return ((8*header_.sub_chank2_size_)/(header_.number_of_channels_*header_.bits_per_sample_));
}

void WaveReader::ReadMainChank() {
    ReadXBitToString(header_.chunk_id_, 4);
    ReadSingleType<uint32_t >(&header_.chunk_size_);
    ReadXBitToString(header_.format_, 4);
}

void WaveReader::ReadSubChank1() {
    ReadXBitToString(header_.sub_chank1_id_, 4);
    ReadSingleType<uint32_t >(&header_.chunk1_size_);
    ReadSingleType<uint16_t >(&header_.format_code_);
    ReadSingleType<uint16_t >(&header_.number_of_channels_);
    ReadSingleType<uint32_t >(&header_.samples_per_second_);
    ReadSingleType<uint32_t >(&header_.bytes_per_second_);
    ReadSingleType<uint16_t >(&header_.bytes_per_sample_frame_);
    ReadSingleType<uint16_t >(&header_.bits_per_sample_);
}

void WaveReader::ReadSubChank2() {
    ReadXBitToString(header_.sub_chank2_id_, 4);
    if (header_.bytes_per_sample_frame_ == 4)
        ReadSingleType<uint32_t >(&header_.sub_chank2_size_);
    else if (header_.bytes_per_sample_frame_ == 2) {
        uint16_t tmp;
        ReadSingleType<uint16_t >(&tmp);
        header_.sub_chank2_size_ = tmp;
    }
    header_.number_of_samples_ = CalculateSamplesNumber();
    ReadSamples(header_.number_of_samples_, header_.number_of_channels_);
}

void WaveReader::SaveSamplesToFile() {
    std::ofstream myfile("samples.txt");
    std::copy(samples_.begin(), samples_.end(), std::ostream_iterator<int16_t >(myfile, "\n"));
}

void WaveReader::CalculateFFT() {
    int size = 44100;
    std::fstream file("samples.txt",std::ios::in);
    if (file.is_open()) {
        double sample;
        Complex v[size];
        for (int i = 0; i<size; i++) {
            file >> sample;
            v[i] = sample;
        }

        CArray data(v, size);

        // forward fft
        fft(data);

        std::vector<double> vec;
        for(auto dat : data) {
            vec.emplace_back(std::abs(dat));
        }

        std::ofstream myfile("sample1fft.txt");
        std::copy(vec.begin(), vec.end()-vec.size()/2, std::ostream_iterator<double>(myfile, "\n"));
    }
}