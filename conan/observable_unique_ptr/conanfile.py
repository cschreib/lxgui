import os
import shutil

from conan import ConanFile
from conan.tools.files import copy, download, unzip
from conan.tools.layout import basic_layout
from conan.tools.build import check_min_cppstd

required_conan_version = ">=2.1"


class OupConan(ConanFile):
    name = "observable_unique_ptr"
    description = "Unique-ownership smart pointers with observable lifetime."
    license = "MIT"
    url = "https://github.com/cschreib/observable_unique_ptr"
    homepage = "https://github.com/cschreib/observable_unique_ptr"
    topics = ("format", "iostream", "printf")
    package_type = "header-library"
    exports_sources = "include/*"

    def set_version(self):
        if not self.version:
            self.version = '0.7.2'

    def layout(self):
        basic_layout(self, src_folder="src")

    def validate(self):
        if self.settings.get_safe("compiler.cppstd"):
            check_min_cppstd(self, 17)

    def source(self):
        url = f'https://github.com/cschreib/observable_unique_ptr/archive/refs/tags/v{self.version}.zip'
        base_name = f'observable_unique_ptr-{self.version}'
        zip_name = f'{base_name}.zip'
        download(self, url, zip_name)
        unzip(self, zip_name)
        shutil.move(os.path.join(base_name, "include"), "include")
        shutil.move(os.path.join(base_name, "LICENSE"), "LICENSE")
        os.unlink(zip_name)

    def package(self):
        copy(self, pattern="LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        copy(self, pattern="*.hpp", src=os.path.join(self.source_folder, "include"), dst=os.path.join(self.package_folder, "include"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "oup")
        self.cpp_info.set_property("cmake_target_name", "oup::oup")
        self.cpp_info.set_property("pkg_config_name",  "oup")
