#
# Be sure to run `pod lib lint Swocket.podspec' to ensure this is a
# valid spec and remove all comments before submitting the spec.
#
# Any lines starting with a # are optional, but encouraged
#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = "Swocket"
  s.version          = "0.1.0"
  s.summary          = "Asynchronous network framework in Swift"
  s.homepage         = "https://github.com/mikaoj/Swocket"
  s.license          = 'MIT'
  s.author           = { "Joakim Gyllström" => "joakim@backslashed.se" }
  s.source           = { :git => "https://github.com/mikaoj/Swocket.git", :tag => s.version.to_s }
  s.ios.deployment_target = '8.0'
  s.osx.deployment_target = '10.10'
  s.requires_arc = true

  # The base 
  s.subspec 'Base' do |base|
    base.source_files = 'Pod/Swocket/**/*'
  end

  # Asynchronous 
  s.subspec 'Async' do |async|
    async.dependency 'Swocket/Base'
    async.dependency 'BrightFutures'
    async.source_files = 'Pod/Async/**/*'
  end

end
