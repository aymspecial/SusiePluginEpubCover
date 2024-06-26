# Epub Cover Thumbnail for Susie Plug-in

## Summary
Epub 書籍ファイルの内部にある表紙（カバー）の画像ファイルを表示するための Susie プラグインです。

## Motivations
フリーソフト FenrirFS (https://www.fenrir-inc.com/jp/fenrirfs/) にて Epub 書籍ファイルのサムネイルを表示するために作成しました。

## Requirement
製作環境：OS:Windows 11 Pro 23H2、FenrirFS v2.7.2、Visual Studio 2022 Community Edition 17.9.6 (Win32,x86)

## Install & Usage
Susie プラグインの使用法に従って下さい。<br/>
FenrirFS で使用する場合は EpubCover.spi を<br/>
<br/>
<ユーザーパス>\AppData\Roaming\Fenrir Inc\FenrirFS\plugins\susie<br/>
<br/>
にコピーし、FenrirFS を再起動することで epub 書籍ファイルのサムネイルを表示します。<br/>
## Features
・対応する拡張子は epub です。<br/>
・対応する epub 内部の画像ファイルの拡張子は png, jpg, jpeg, gif, tiff です。<br/>
・epub 書庫内のインデックスファイル *.opf ファイルの中を検索し、表紙のタグが付いている画像ファイルを GetPicture で返します。<br/>
・表紙として定義されているファイルが無い場合、内部画像ファイルの中から一番トップのファイルを返す作りとなっています。

## Reference
・「Susieプラグインの制作方法」 (https://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html)<br/>
・「Susieプラグインをつくる」 (http://elksimple.web.fc2.com/memo/if_spi.html)<br/>
・stb (https://github.com/nothings/stb)<br/>
・minizip (https://github.com/domoticz/minizip)<br/>
・zlib 1.3.1 (https://www.zlib.net/)<br/>
・tinyxml2 (https://github.com/leethomason/tinyxml2) <br/>

## Updates
v0.0.2 - 書庫内の opf ファイルの中身を見て表紙画像ファイル名を取得するようにした。

## Author
Kax Aym (aymspecial@msn.com)

## License
MIT License を採用します。
改善する場合は Pull Request を当方 (aymspecial@msn.com) に頂けると非常に助かりますが、あくまでも希望です。
