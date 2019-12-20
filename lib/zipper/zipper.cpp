/********************************************************************************
 *   Author(s):                                                                 *
 *              M.Khaled  <eng.mk@msn.com>                                      *
 ********************************************************************************/

#include <iostream>
#include <sstream>
#include "miniz.h"
#include "zipper.h"

#define ZIPPER_ZIP_FILE_COMMENT "Zipped with Zipper!"

void Zipper::compressFiles(const std::vector<std::string>& fileNames, const std::string targetFile, int level)
{
    std::string comment = ZIPPER_ZIP_FILE_COMMENT;
    
    // make a zip archive from miniz
    mz_zip_archive zip_archive;

    //set the zip archive to empty
    memset(&zip_archive, 0, sizeof(zip_archive));

    //initialize for writing
    mz_bool result;
    result = mz_zip_writer_init_file(&zip_archive, targetFile.c_str(), 0);
    if (!result) {
        std::stringstream err;
        err << "Zipper::compress: Failed to initialize the file (" << targetFile
            << ") in the archieve for compressing: ";
        err << mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)) << std::endl;
        std::string err_msg = err.str();
        throw std::runtime_error(err_msg);
    }

    // for each file    
    for (size_t i = 0; i < fileNames.size(); i++){    
        
        std::string filename = fileNames[i];

        //add the file to the archive
        result = mz_zip_writer_add_file(&zip_archive, filename.c_str(), filename.c_str(), 
            comment.c_str(), (mz_uint16)comment.length(), level);
        if (!result){
            std::stringstream err;
            err << "Zipper::compress:Error while adding a zip file (" << filename
                << ") to zip archive: ";
            err << mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)) << std::endl;
            std::string err_msg = err.str();
            throw std::runtime_error(err_msg);
        }
    }

    // finalize the archieve
    result = mz_zip_writer_finalize_archive(&zip_archive);
    if (!result){
        std::stringstream err;
        err << "Zipper::compress: Error while finalizing zip archive: ";
        err << mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)) << std::endl;
        std::string err_msg = err.str();
        throw std::runtime_error(err_msg);
    }
}


std::vector<std::string> Zipper::uncompressFile(const std::string& filename, const std::string& destDir)
{
    std::vector<std::string> fileNames;

    // make a zip archive from miniz
    mz_zip_archive zip_archive;

    //set the zip archive to empty
    memset(&zip_archive, 0, sizeof(zip_archive));

    // initalize for reading
    mz_bool result = mz_zip_reader_init_file(&zip_archive, filename.c_str(), 0);
    if (!result){
        std::stringstream err;
        err << "Zipper::uncompress: Failed to initialize the file (" << filename
            << ") in the archieve for uncompressing: ";
        err << mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)) << std::endl;
        std::string err_msg = err.str();
        throw std::runtime_error(err_msg);
    }

    mz_uint count = mz_zip_reader_get_num_files(&zip_archive);
    if (count == 0){
        mz_zip_reader_end(&zip_archive);
        return fileNames;
    }

    // extract the files
    for (mz_uint i = 0; i < count; ++i){
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
            continue;

        // skip directories
        if (mz_zip_reader_is_file_a_directory(&zip_archive, i))
            continue;

        std::string destFileName = (destDir.empty()? ".": destDir) + std::string(file_stat.m_filename);
        fileNames.push_back(destFileName);

        result = mz_zip_reader_extract_to_file(&zip_archive, i, destFileName.c_str(), 0);
        if (!result){
            std::stringstream err;
            err << "Zipper::uncompress: Error while extracting file (" << destFileName
                << ") from zip archive (" << filename
                << "): ";
            err << mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)) << std::endl;
            std::string err_msg = err.str();
            throw std::runtime_error(err_msg);
        }
    }

    // close the archive
    mz_zip_reader_end(&zip_archive);

    return fileNames;
}